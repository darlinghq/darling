/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * SIOCGIFCONF on a socket: the list holds, per interface, a link-level
 * record (AF_LINK) followed by one record per address. Records are
 * IFNAMSIZ bytes of name plus the address, at least sizeof(struct sockaddr)
 * long, and are packed back to back. A short buffer receives only the whole
 * records that fit, ifc_len reports the bytes written, and the call still
 * succeeds. A NULL buffer is EFAULT unless the length is zero.
 *
 * Runs on macOS and in Darling; prints PASS and exits 0.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <netinet/in.h>

#define CHECK(cond) do { if (!(cond)) { \
	printf("FAIL: %s (line %d, errno %d)\n", #cond, __LINE__, errno); exit(1); } } while (0)

#define MAX_RECORDS 512
#define BIG (256 * 1024)

static size_t record_length(const struct ifreq *r)
{
	size_t sa = r->ifr_addr.sa_len;
	return IFNAMSIZ + (sa < sizeof(struct sockaddr) ? sizeof(struct sockaddr) : sa);
}

int main(void)
{
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	CHECK(s >= 0);

	struct ifconf c;
	c.ifc_len = 0;
	c.ifc_buf = NULL;
	CHECK(ioctl(s, SIOCGIFCONF, &c) == 0);
	CHECK(c.ifc_len == 0);

	c.ifc_len = 64;
	c.ifc_buf = NULL;
	CHECK(ioctl(s, SIOCGIFCONF, &c) == -1 && errno == EFAULT);

	char *buf = calloc(1, BIG);
	CHECK(buf);
	c.ifc_len = BIG;
	c.ifc_buf = buf;
	CHECK(ioctl(s, SIOCGIFCONF, &c) == 0);
	int total = c.ifc_len;
	CHECK(total > 0 && total < BIG);

	// Walk the records; remember where each one ends.
	int ends[MAX_RECORDS];
	int nrec = 0, inet = 0, loopback_link = 0;
	char seen_links[64][IFNAMSIZ];
	int nlinks = 0;
	char current[IFNAMSIZ] = "";
	int have_link = 0;
	for (int off = 0; off < total; ) {
		CHECK(nrec < MAX_RECORDS);
		struct ifreq *r = (struct ifreq *)(buf + off);
		size_t len = record_length(r);
		CHECK(off + (int)len <= total);
		CHECK(strnlen(r->ifr_name, IFNAMSIZ) < IFNAMSIZ);
		if (r->ifr_addr.sa_family == AF_LINK) {
			struct sockaddr_dl *dl = (struct sockaddr_dl *)&r->ifr_addr;
			CHECK(dl->sdl_len >= sizeof(struct sockaddr_dl));
			CHECK(dl->sdl_nlen == strlen(r->ifr_name));
			CHECK(memcmp(dl->sdl_data, r->ifr_name, dl->sdl_nlen) == 0);
			CHECK(dl->sdl_index > 0);
			// One link record per interface.
			for (int i = 0; i < nlinks; i++)
				CHECK(strcmp(seen_links[i], r->ifr_name) != 0);
			CHECK(nlinks < 64);
			strcpy(seen_links[nlinks++], r->ifr_name);
			strcpy(current, r->ifr_name);
			have_link = 1;
			if (dl->sdl_type == IFT_LOOP && strncmp(r->ifr_name, "lo", 2) == 0)
				loopback_link = 1;
			if (dl->sdl_type == IFT_ETHER)
				CHECK(dl->sdl_alen == 6);
		} else {
			// Address records follow their interface's link record.
			CHECK(have_link);
			CHECK(strcmp(current, r->ifr_name) == 0);
			if (r->ifr_addr.sa_family == AF_INET) {
				CHECK(r->ifr_addr.sa_len == sizeof(struct sockaddr_in));
				inet++;
			}
		}
		off += len;
		ends[nrec++] = off;
	}
	CHECK(inet > 0);
	CHECK(loopback_link);

	// Short buffers: only whole records, ifc_len = bytes written, no overrun.
	// test_quirk_siocgifconf_address_record_needs_spare_byte: an address
	// record is written only when more than its own length remains, whereas a
	// link-level record needs exactly its length. A longer (IPv6) record may
	// be cut short by the system; Darling only lists IPv4, so that is allowed
	// to differ.
	char *small = malloc(total + 64);
	CHECK(small);
	int previous = 0;
	for (int len = 0; len <= total + 8; len++) {
		memset(small, 0xAA, total + 64);
		c.ifc_len = len;
		c.ifc_buf = small;
		CHECK(ioctl(s, SIOCGIFCONF, &c) == 0);
		CHECK(c.ifc_len <= len);
		CHECK(c.ifc_len >= previous);
		previous = c.ifc_len;
		int whole = 0;
		for (int i = 0; i < nrec && ends[i] <= c.ifc_len; i++)
			whole = ends[i];
		CHECK(memcmp(small, buf, whole) == 0);
		for (int i = len; i < total + 64; i++)
			CHECK((unsigned char)small[i] == 0xAA);

		int boundary = (c.ifc_len == 0);
		int partial_inet6 = 0;
		int start = 0;
		for (int i = 0; i < nrec; i++) {
			struct ifreq *r = (struct ifreq *)(buf + start);
			int family = r->ifr_addr.sa_family;
			int included = c.ifc_len >= ends[i];
			int fits = family == AF_LINK ? len >= ends[i] : len > ends[i];
			if (c.ifc_len == ends[i])
				boundary = 1;
			if (c.ifc_len > start && c.ifc_len < ends[i] && family == AF_INET6)
				partial_inet6 = 1;
			if (family == AF_LINK || family == AF_INET) {
				// A record is listed when it fits and the one before it was listed.
				int before = i == 0 || c.ifc_len >= start;
				if (!fits)
					CHECK(!included);
				else if (before)
					CHECK(included);
			}
			start = ends[i];
		}
		CHECK(boundary || partial_inet6);
	}
	CHECK(previous == total);

	printf("PASS: SIOCGIFCONF layout and length semantics\n");
	return 0;
}
