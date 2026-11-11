// Copyright (C) 2026 Lubos Dolezel
//
// /dev/bpf on the loopback interface, as bpf(4) describes it: attach to
// lo0 (Darling shows Linux interface names, so "lo" is tried as well),
// filter UDP to one port, send datagrams to that port and another one, and
// check the records read back: struct bpf_hdr fields, the DLT_NULL header,
// the payload, that the other port is filtered out, that each packet is seen
// once, BIOCGSTATS, the read timeout, immediate mode, non-blocking reads,
// EINVAL for a read size other than the buffer size, ENXIO for a read before
// an interface is attached, and EBUSY for a second open of the same unit.
//
// Needs root (CAP_NET_RAW on Linux). Without it, open() must fail with
// EACCES; the test then prints "SKIP (not root)" and exits 0.

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <net/bpf.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define PORT_WANTED 47361
#define PORT_OTHER 47362
#define NSEND 3
#define BUFLEN 32768

static int failures;

#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL %s:%d: ", __func__, __LINE__); printf(__VA_ARGS__); printf("\n"); failures++; } } while (0)

static double now_ms(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

static int udp_bound(int port)
{
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	if (s < 0 || bind(s, (struct sockaddr*) &sin, sizeof(sin)) != 0)
	{
		perror("bind udp");
		exit(1);
	}
	return s;
}

static void send_to(int s, int port, const char* payload)
{
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	if (sendto(s, payload, strlen(payload), 0, (struct sockaddr*) &sin, sizeof(sin)) < 0)
	{
		perror("sendto");
		exit(1);
	}
}

// "ip and udp and dst port PORT_WANTED" for DLT_NULL. The link header is the
// address family in host byte order, which a 32-bit BPF load (big-endian)
// sees as ntohl(AF_INET).
static struct bpf_insn filter_insns[] = {
	BPF_STMT(BPF_LD | BPF_W | BPF_ABS, 0),
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 0, 0, 6), // k patched in main
	BPF_STMT(BPF_LD | BPF_B | BPF_ABS, 4 + 9),
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, IPPROTO_UDP, 0, 4),
	BPF_STMT(BPF_LDX | BPF_B | BPF_MSH, 4),
	BPF_STMT(BPF_LD | BPF_H | BPF_IND, 4 + 2),
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, PORT_WANTED, 0, 1),
	BPF_STMT(BPF_RET | BPF_K, 0xffff),
	BPF_STMT(BPF_RET | BPF_K, 0),
};

struct seen {
	int count[NSEND];
	int other;
	int records;
};

// Parses a buffer returned by read(); checks every record.
static void parse(const unsigned char* buf, ssize_t n, struct seen* seen)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	ssize_t off = 0;
	while (off < n)
	{
		CHECK(off + (ssize_t) sizeof(struct bpf_hdr) - 2 <= n, "truncated header at %zd of %zd", off, n);
		if (off + (ssize_t) sizeof(struct bpf_hdr) - 2 > n)
			return;
		const struct bpf_hdr* bh = (const struct bpf_hdr*) (buf + off);
		seen->records++;

		CHECK(bh->bh_hdrlen >= 18, "bh_hdrlen %u", bh->bh_hdrlen);
		// The network header after the 4-byte DLT_NULL header is aligned.
		CHECK((bh->bh_hdrlen + 4) % 4 == 0, "bh_hdrlen %u leaves the IP header unaligned", bh->bh_hdrlen);
		CHECK(bh->bh_caplen == bh->bh_datalen, "caplen %u datalen %u", bh->bh_caplen, bh->bh_datalen);
		CHECK(off + bh->bh_hdrlen + bh->bh_caplen <= n, "record runs past the data");
		long age = (long) tv.tv_sec - (long) bh->bh_tstamp.tv_sec;
		CHECK(age >= -1 && age <= 10, "timestamp %ld s away from now", age);
		CHECK(bh->bh_tstamp.tv_usec >= 0 && bh->bh_tstamp.tv_usec < 1000000, "tv_usec %d", (int) bh->bh_tstamp.tv_usec);

		const unsigned char* pkt = buf + off + bh->bh_hdrlen;
		uint32_t family;
		memcpy(&family, pkt, 4);
		CHECK(family == AF_INET, "DLT_NULL family %u", family);

		const unsigned char* ip = pkt + 4;
		unsigned ihl = (ip[0] & 0x0f) * 4;
		CHECK((ip[0] >> 4) == 4 && ip[9] == IPPROTO_UDP, "not IPv4/UDP");
		const unsigned char* udp = ip + ihl;
		unsigned dport = (udp[2] << 8) | udp[3];
		unsigned plen = bh->bh_caplen - 4 - ihl - 8;

		if (dport == PORT_WANTED)
		{
			int idx = -1;
			char payload[64] = { 0 };
			if (plen < sizeof(payload))
				memcpy(payload, udp + 8, plen);
			if (sscanf(payload, "bpf-test-%d", &idx) == 1 && idx >= 0 && idx < NSEND)
				seen->count[idx]++;
			else
				CHECK(0, "unexpected payload '%s'", payload);
			CHECK(bh->bh_datalen == 4 + ihl + 8 + plen, "datalen %u", bh->bh_datalen);
		}
		else
			seen->other++;

		off += BPF_WORDALIGN(bh->bh_hdrlen + bh->bh_caplen);
	}
}

int main(void)
{
	int fd = open("/dev/bpf0", O_RDWR);
	if (fd < 0)
	{
		int err = errno;
		if (geteuid() != 0)
		{
			if (err == EACCES)
			{
				printf("open(/dev/bpf0) failed with EACCES as expected\n");
				printf("SKIP (not root)\n");
				return 0;
			}
			printf("FAIL: open(/dev/bpf0) as non-root: %s, expected EACCES\n", strerror(err));
			return 1;
		}
		printf("FAIL: open(/dev/bpf0) as root: %s\n", strerror(err));
		return 1;
	}

	struct stat st;
	CHECK(fstat(fd, &st) == 0 && S_ISCHR(st.st_mode), "fstat: not a character device (mode %o)", st.st_mode);

	// A unit can be open only once.
	int fd2 = open("/dev/bpf0", O_RDWR);
	CHECK(fd2 == -1 && errno == EBUSY, "second open of /dev/bpf0: %d (%s), expected EBUSY", fd2, strerror(errno));
	if (fd2 >= 0)
		close(fd2);

	struct bpf_version bv;
	CHECK(ioctl(fd, BIOCVERSION, &bv) == 0 && bv.bv_major == BPF_MAJOR_VERSION && bv.bv_minor >= BPF_MINOR_VERSION,
		"BIOCVERSION");

	// Reading before an interface is attached.
	u_int blen = 0;
	CHECK(ioctl(fd, BIOCGBLEN, &blen) == 0 && blen > 0, "BIOCGBLEN");
	u_int dlt;
	CHECK(ioctl(fd, BIOCGDLT, &dlt) == -1 && errno == EINVAL, "BIOCGDLT before BIOCSETIF should be EINVAL");
	unsigned char* early = malloc(blen);
	CHECK(read(fd, early, blen) == -1 && errno == ENXIO, "read before BIOCSETIF should be ENXIO");
	free(early);

	blen = BUFLEN;
	CHECK(ioctl(fd, BIOCSBLEN, &blen) == 0 && blen == BUFLEN, "BIOCSBLEN %u", blen);

	struct ifreq ifr;
	const char* ifname = "lo0";
	memset(&ifr, 0, sizeof(ifr));
	strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (ioctl(fd, BIOCSETIF, &ifr) != 0)
	{
		CHECK(errno == ENXIO, "BIOCSETIF lo0: %s, expected ENXIO for a missing interface", strerror(errno));
		ifname = "lo";
		strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
		if (ioctl(fd, BIOCSETIF, &ifr) != 0)
		{
			printf("FAIL: BIOCSETIF lo0/lo: %s\n", strerror(errno));
			return 1;
		}
	}

	memset(&ifr, 0, sizeof(ifr));
	CHECK(ioctl(fd, BIOCGETIF, &ifr) == 0 && strcmp(ifr.ifr_name, ifname) == 0, "BIOCGETIF '%s'", ifr.ifr_name);

	CHECK(ioctl(fd, BIOCGDLT, &dlt) == 0 && dlt == DLT_NULL, "BIOCGDLT %u, expected DLT_NULL", dlt);

	u_int dlts[8];
	struct bpf_dltlist list;
	memset(&list, 0, sizeof(list));
	list.bfl_len = 8;
	list.bfl_list = dlts;
	int has_null = 0;
	CHECK(ioctl(fd, BIOCGDLTLIST, &list) == 0 && list.bfl_len >= 1, "BIOCGDLTLIST");
	for (u_int i = 0; i < list.bfl_len && i < 8; i++)
		has_null |= dlts[i] == DLT_NULL;
	CHECK(has_null, "BIOCGDLTLIST lacks DLT_NULL");

	blen = 4096;
	CHECK(ioctl(fd, BIOCSBLEN, &blen) == -1 && errno == EINVAL, "BIOCSBLEN after BIOCSETIF should be EINVAL");
	CHECK(ioctl(fd, BIOCGBLEN, &blen) == 0 && blen == BUFLEN, "BIOCGBLEN %u after BIOCSETIF", blen);

	// A program that does not end in a return is refused.
	struct bpf_insn bad[] = { BPF_STMT(BPF_LD | BPF_W | BPF_ABS, 0) };
	struct bpf_program badprog = { 1, bad };
	CHECK(ioctl(fd, BIOCSETF, &badprog) == -1 && errno == EINVAL, "BIOCSETF without RET should be EINVAL");

	filter_insns[1].k = ntohl(AF_INET);
	struct bpf_program prog = { sizeof(filter_insns) / sizeof(filter_insns[0]), filter_insns };
	CHECK(ioctl(fd, BIOCSETF, &prog) == 0, "BIOCSETF: %s", strerror(errno));

	struct timeval to = { 0, 300000 };
	CHECK(ioctl(fd, BIOCSRTIMEOUT, &to) == 0, "BIOCSRTIMEOUT");
	struct timeval got_to;
	CHECK(ioctl(fd, BIOCGRTIMEOUT, &got_to) == 0 && got_to.tv_sec == 0 && got_to.tv_usec > 0 && got_to.tv_usec <= 300000,
		"BIOCGRTIMEOUT %ld.%06d", (long) got_to.tv_sec, (int) got_to.tv_usec);
	struct timeval bad_to = { 0, 1000000 };
	CHECK(ioctl(fd, BIOCSRTIMEOUT, &bad_to) == -1 && errno == EINVAL, "BIOCSRTIMEOUT with tv_usec 1000000 should be EINVAL");

	unsigned char* buf = malloc(BUFLEN);

	// Read size must equal the buffer size.
	CHECK(read(fd, buf, BUFLEN - 1) == -1 && errno == EINVAL, "read with a wrong size should be EINVAL");

	// Nothing matches: the read returns 0 bytes once the timeout expires.
	CHECK(ioctl(fd, BIOCFLUSH) == 0, "BIOCFLUSH");
	double t0 = now_ms();
	ssize_t n = read(fd, buf, BUFLEN);
	double dt = now_ms() - t0;
	CHECK(n == 0, "idle read returned %zd (%s)", n, n < 0 ? strerror(errno) : "");
	CHECK(dt >= 200 && dt < 3000, "idle read took %.0f ms, expected about 300", dt);

	// Non-blocking with nothing queued: EWOULDBLOCK.
	int on = 1, off = 0;
	CHECK(ioctl(fd, FIONBIO, &on) == 0, "FIONBIO");
	CHECK(read(fd, buf, BUFLEN) == -1 && errno == EWOULDBLOCK, "non-blocking idle read should be EWOULDBLOCK");
	CHECK(ioctl(fd, FIONBIO, &off) == 0, "FIONBIO off");

	int rx_wanted = udp_bound(PORT_WANTED);
	int rx_other = udp_bound(PORT_OTHER);
	int tx = socket(AF_INET, SOCK_DGRAM, 0);

	// Not immediate: the read waits for the timeout, then returns the
	// buffered packets.
	char payload[32];
	for (int i = 0; i < NSEND; i++)
	{
		snprintf(payload, sizeof(payload), "bpf-test-%d", i);
		send_to(tx, PORT_WANTED, payload);
		send_to(tx, PORT_OTHER, "bpf-other");
	}

	struct seen seen;
	memset(&seen, 0, sizeof(seen));
	t0 = now_ms();
	n = read(fd, buf, BUFLEN);
	dt = now_ms() - t0;
	CHECK(n > 0, "read returned %zd (%s)", n, n < 0 ? strerror(errno) : "");
	CHECK(dt >= 200, "non-immediate read returned after %.0f ms, before the timeout", dt);
	if (n > 0)
		parse(buf, n, &seen);
	// Pick up anything that arrived after the first read.
	for (int tries = 0; tries < 3; tries++)
	{
		int total = 0;
		for (int i = 0; i < NSEND; i++)
			total += seen.count[i];
		if (total >= NSEND)
			break;
		n = read(fd, buf, BUFLEN);
		if (n > 0)
			parse(buf, n, &seen);
	}
	for (int i = 0; i < NSEND; i++)
		CHECK(seen.count[i] == 1, "packet %d seen %d times", i, seen.count[i]);
	CHECK(seen.other == 0, "%d packets to the other port passed the filter", seen.other);

	struct bpf_stat bs;
	CHECK(ioctl(fd, BIOCGSTATS, &bs) == 0, "BIOCGSTATS");
	CHECK(bs.bs_recv >= NSEND, "bs_recv %u", bs.bs_recv);

	// Immediate mode: a read returns as soon as a packet is there.
	u_int imm = 1;
	CHECK(ioctl(fd, BIOCIMMEDIATE, &imm) == 0, "BIOCIMMEDIATE");
	send_to(tx, PORT_WANTED, "bpf-test-0");
	usleep(20000);
	memset(&seen, 0, sizeof(seen));
	t0 = now_ms();
	n = read(fd, buf, BUFLEN);
	dt = now_ms() - t0;
	CHECK(n > 0, "immediate read returned %zd", n);
	CHECK(dt < 150, "immediate read took %.0f ms", dt);
	if (n > 0)
		parse(buf, n, &seen);
	CHECK(seen.count[0] == 1 && seen.records == 1, "immediate read: %d records", seen.records);

	close(tx);
	close(rx_wanted);
	close(rx_other);
	close(fd);
	free(buf);

	if (failures)
	{
		printf("FAIL: %d check(s) failed\n", failures);
		return 1;
	}
	printf("PASS: /dev/bpf capture on %s\n", ifname);
	return 0;
}
