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

// libpcap (/usr/lib/libpcap.A.dylib) through its public, file-based API:
// version string, writing a savefile with pcap_dump(), reading it back with
// pcap_open_offline()/pcap_fopen_offline(), compiling a filter with
// pcap_compile() and running it with pcap_offline_filter(). Needs no capture
// privileges. Live capture is only checked to return a handle or fail with
// a message.
// Build: cc -o pcap_savefile pcap_savefile.c -lpcap

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pcap/pcap.h>

static int failures;

#define CHECK(cond, ...) do { \
	if (cond) { printf("PASS "); } else { printf("FAIL "); failures++; } \
	printf(__VA_ARGS__); printf("\n"); \
} while (0)

// Ethernet + IPv4 + TCP/UDP headers, ports 1234 -> 80, no payload
#define FRAME_LEN (14 + 20 + 20)

static void make_frame(u_char* f, int tcp)
{
	memset(f, 0, FRAME_LEN);
	memcpy(f, "\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa\xbb\x08\x00", 14);
	u_char* ip = f + 14;
	ip[0] = 0x45; // IPv4, 20-byte header
	ip[2] = 0; ip[3] = 40; // total length
	ip[8] = 64; // TTL
	ip[9] = tcp ? 6 : 17; // protocol
	memcpy(ip + 12, "\x0a\x00\x00\x01\x0a\x00\x00\x02", 8);
	u_char* l4 = ip + 20;
	l4[0] = 1234 >> 8; l4[1] = 1234 & 0xff;
	l4[2] = 0; l4[3] = 80;
	if (tcp)
		l4[12] = 5 << 4; // data offset
	else
		l4[5] = 8 + 12; // UDP length (fits the TCP-sized slot)
}

static int header_ok(const struct pcap_pkthdr* h, int index)
{
	return h->ts.tv_sec == 1234 + index && h->ts.tv_usec == 567890
		&& h->caplen == FRAME_LEN && h->len == FRAME_LEN + 6;
}

static int loop_count, loop_good;

static void count_packet(u_char* user, const struct pcap_pkthdr* h, const u_char* bytes)
{
	(void) user;
	(void) bytes;
	if (header_ok(h, loop_count))
		loop_good++;
	loop_count++;
}

int main(void)
{
	char errbuf[PCAP_ERRBUF_SIZE];
	char path[64];
	u_char frames[2][FRAME_LEN];
	const char* version = pcap_lib_version();

	CHECK(version && strncmp(version, "libpcap version ", 16) == 0,
		"pcap_lib_version() = \"%s\"", version ? version : "(null)");

	make_frame(frames[0], 1);
	make_frame(frames[1], 0);

	pcap_t* dead = pcap_open_dead(DLT_EN10MB, 65535);
	CHECK(dead && pcap_datalink(dead) == DLT_EN10MB && pcap_snapshot(dead) == 65535,
		"pcap_open_dead(DLT_EN10MB, 65535)");
	if (!dead)
		return 1;

	struct bpf_program bad;
	CHECK(pcap_compile(dead, &bad, "bogus filter (", 1, 0) == -1 && pcap_geterr(dead)[0],
		"pcap_compile() of a bad filter fails with a message");

	// Write a savefile: a TCP and a UDP frame to port 80
	snprintf(path, sizeof(path), "/tmp/pcap_savefile.%d.pcap", (int) getpid());
	pcap_dumper_t* dumper = pcap_dump_open(dead, path);
	CHECK(dumper != NULL, "pcap_dump_open(%s)", path);
	if (!dumper)
		return 1;
	for (int i = 0; i < 2; i++)
	{
		struct pcap_pkthdr h;
		memset(&h, 0, sizeof(h));
		h.ts.tv_sec = 1234 + i;
		h.ts.tv_usec = 567890;
		h.caplen = FRAME_LEN;
		h.len = FRAME_LEN + 6;
		pcap_dump((u_char*) dumper, &h, frames[i]);
	}
	CHECK(pcap_dump_flush(dumper) == 0, "pcap_dump_flush()");
	pcap_dump_close(dumper);
	pcap_close(dead);

	// Classic pcap format: 24-byte file header, 16-byte record headers
	struct stat st;
	CHECK(stat(path, &st) == 0 && st.st_size == 24 + 2 * (16 + FRAME_LEN),
		"savefile has the file header and two records");

	pcap_t* offline = pcap_open_offline(path, errbuf);
	CHECK(offline != NULL, "pcap_open_offline(%s)", path);
	if (offline)
	{
		CHECK(pcap_datalink(offline) == DLT_EN10MB && pcap_snapshot(offline) == 65535
			&& pcap_major_version(offline) == 2 && pcap_minor_version(offline) == 4,
			"savefile header: link type, snapshot length, version 2.4");

		struct bpf_program tcp80, udp;
		int ok = pcap_compile(offline, &tcp80, "tcp dst port 80", 1, PCAP_NETMASK_UNKNOWN) == 0
			&& pcap_compile(offline, &udp, "udp", 1, PCAP_NETMASK_UNKNOWN) == 0;
		CHECK(ok && tcp80.bf_len > 0, "pcap_compile(\"tcp dst port 80\"), pcap_compile(\"udp\")");

		struct pcap_pkthdr* h = NULL;
		const u_char* data = NULL;
		CHECK(pcap_next_ex(offline, &h, &data) == 1 && header_ok(h, 0)
			&& memcmp(data, frames[0], FRAME_LEN) == 0,
			"pcap_next_ex(): first record's header and data");
		if (ok && data)
			CHECK(pcap_offline_filter(&tcp80, h, data) != 0 && pcap_offline_filter(&udp, h, data) == 0,
				"pcap_offline_filter(): TCP frame matches \"tcp dst port 80\", not \"udp\"");

		struct pcap_pkthdr own;
		data = pcap_next(offline, &own);
		CHECK(data && header_ok(&own, 1) && memcmp(data, frames[1], FRAME_LEN) == 0,
			"pcap_next(): second record");
		if (ok && data)
			CHECK(pcap_offline_filter(&tcp80, &own, data) == 0 && pcap_offline_filter(&udp, &own, data) != 0,
				"pcap_offline_filter(): UDP frame matches \"udp\", not \"tcp dst port 80\"");

		CHECK(pcap_next_ex(offline, &h, &data) == PCAP_ERROR_BREAK, "pcap_next_ex() at the end of the file");
		if (ok)
		{
			pcap_freecode(&tcp80);
			pcap_freecode(&udp);
		}
		pcap_close(offline);
	}

	// pcap_setfilter() on a savefile filters what pcap_loop() delivers
	offline = pcap_open_offline(path, errbuf);
	if (offline)
	{
		struct bpf_program tcp;
		int rv = pcap_compile(offline, &tcp, "tcp", 1, PCAP_NETMASK_UNKNOWN);
		rv = rv == 0 ? pcap_setfilter(offline, &tcp) : rv;
		if (rv == 0)
			pcap_freecode(&tcp);
		rv = rv == 0 ? pcap_loop(offline, -1, count_packet, NULL) : rv;
		CHECK(rv == 0 && loop_count == 1 && loop_good == 1,
			"pcap_setfilter(\"tcp\") + pcap_loop() delivers only the TCP record");
		pcap_close(offline);
	}

	FILE* fp = fopen(path, "rb");
	offline = fp ? pcap_fopen_offline(fp, errbuf) : NULL;
	CHECK(offline != NULL, "pcap_fopen_offline() on a FILE");
	if (offline)
	{
		struct pcap_pkthdr* h = NULL;
		const u_char* data = NULL;
		CHECK(pcap_next_ex(offline, &h, &data) == 1 && header_ok(h, 0),
			"pcap_next_ex() after pcap_fopen_offline()");
		pcap_close(offline);
	}
	else if (fp)
		fclose(fp);
	unlink(path);

	errbuf[0] = 0;
	CHECK(pcap_open_offline("/nonexistent/pcap_savefile.pcap", errbuf) == NULL && errbuf[0],
		"pcap_open_offline() of a missing file fails with a message");

	// Live capture needs a BPF device and privileges; it must either work
	// or fail with a message.
	errbuf[0] = 0;
	pcap_t* live = pcap_open_live("lo0", 128, 0, 100, errbuf);
	if (live)
	{
		CHECK(1, "pcap_open_live(lo0) opened a handle");
		pcap_close(live);
	}
	else
		CHECK(errbuf[0] != 0, "pcap_open_live(lo0) fails with a message: %s", errbuf);

	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures ? 1 : 0;
}
