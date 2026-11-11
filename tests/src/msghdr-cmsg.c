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
 * sendmsg()/recvmsg() ancillary data: descriptor passing, truncation,
 * malformed control buffers and the IPv4/IPv6 control message types, in
 * the Darwin layout (4-byte aligned cmsghdr, Darwin type numbers).
 *
 * Expected values come from black-box runs of the same calls on macOS 27
 * (arm64 and x86_64). The checks marked "[darling]" differ on purpose: when
 * SCM_RIGHTS data is truncated, macOS leaves the descriptors that did not fit
 * open in the receiver, where nothing can find them; Darling closes them.
 */
#define __APPLE_USE_RFC_3542 1
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int failures;

#define CHECK(cond, ...) do { \
	if (cond) printf("PASS: "); else { printf("FAIL: "); failures++; } \
	printf(__VA_ARGS__); printf("\n"); \
} while (0)

static int lowest_free_fd(void)
{
	int fd = dup(0);
	close(fd);
	return fd;
}

static int count_open(int from, int n)
{
	int c = 0;
	for (int fd = from; fd < from + n; fd++)
		if (fcntl(fd, F_GETFD) != -1)
			c++;
	return c;
}

static void close_range_(int from, int n)
{
	for (int fd = from; fd < from + n; fd++)
		close(fd);
}

static ssize_t send_rights(int s, const int* fds, int n)
{
	char buf[CMSG_SPACE(8 * sizeof(int))] __attribute__((aligned(8)));
	struct iovec iov = { "x", 1 };
	struct msghdr m;
	struct cmsghdr* c;

	memset(&m, 0, sizeof(m));
	memset(buf, 0, sizeof(buf));
	m.msg_iov = &iov;
	m.msg_iovlen = 1;
	m.msg_control = buf;
	m.msg_controllen = CMSG_SPACE(n * sizeof(int));
	c = CMSG_FIRSTHDR(&m);
	c->cmsg_level = SOL_SOCKET;
	c->cmsg_type = SCM_RIGHTS;
	c->cmsg_len = CMSG_LEN(n * sizeof(int));
	memcpy(CMSG_DATA(c), fds, n * sizeof(int));
	return sendmsg(s, &m, 0);
}

static ssize_t recv_into(int s, void* control, socklen_t len, struct msghdr* m)
{
	static char data[64];
	static struct iovec iov;

	iov.iov_base = data;
	iov.iov_len = sizeof(data);
	memset(m, 0, sizeof(*m));
	m->msg_iov = &iov;
	m->msg_iovlen = 1;
	m->msg_control = control;
	m->msg_controllen = len;
	return recvmsg(s, m, 0);
}

static int same_file(int a, int b)
{
	struct stat sa, sb;
	if (fstat(a, &sa) != 0 || fstat(b, &sb) != 0)
		return 0;
	return sa.st_dev == sb.st_dev && sa.st_ino == sb.st_ino;
}

static void test_rights(void)
{
	int sv[2], fds[6];
	char buf[256] __attribute__((aligned(8)));
	struct msghdr m;
	struct cmsghdr* c;
	ssize_t r;
	int before;

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) != 0)
	{
		CHECK(0, "socketpair: %s", strerror(errno));
		return;
	}
	for (int i = 0; i < 6; i++)
		fds[i] = open("/dev/null", O_RDONLY);

	// Three descriptors in one message.
	CHECK(send_rights(sv[0], fds, 3) == 1, "send 3 fds");
	before = lowest_free_fd();
	r = recv_into(sv[1], buf, sizeof(buf), &m);
	c = CMSG_FIRSTHDR(&m);
	CHECK(r == 1 && c != NULL && c->cmsg_level == SOL_SOCKET && c->cmsg_type == SCM_RIGHTS
			&& c->cmsg_len == CMSG_LEN(3 * sizeof(int)),
		"recv 3 fds: one SCM_RIGHTS, cmsg_len %u", c ? c->cmsg_len : 0);
	CHECK(m.msg_controllen == CMSG_SPACE(3 * sizeof(int)) && !(m.msg_flags & MSG_CTRUNC),
		"msg_controllen %u, flags 0x%x", (unsigned) m.msg_controllen, m.msg_flags);
	if (c != NULL)
	{
		int got[3];
		memcpy(got, CMSG_DATA(c), sizeof(got));
		CHECK(same_file(got[0], fds[0]) && same_file(got[2], fds[2]), "received fds refer to the sent files");
		CHECK(CMSG_NXTHDR(&m, c) == NULL, "no second message");
	}
	close_range_(before, 10);

	// Room for one descriptor out of three.
	CHECK(send_rights(sv[0], fds, 3) == 1, "send 3 fds again");
	before = lowest_free_fd();
	r = recv_into(sv[1], buf, CMSG_SPACE(sizeof(int)), &m);
	c = CMSG_FIRSTHDR(&m);
	CHECK(r == 1 && (m.msg_flags & MSG_CTRUNC), "short buffer: MSG_CTRUNC set (flags 0x%x)", m.msg_flags);
	CHECK(m.msg_controllen <= CMSG_SPACE(sizeof(int)), "msg_controllen %u within the buffer", (unsigned) m.msg_controllen);
	if (c != NULL)
	{
		int got;
		memcpy(&got, CMSG_DATA(c), sizeof(got));
		CHECK(c->cmsg_type == SCM_RIGHTS && got == before && fcntl(got, F_GETFD) != -1,
			"first fd delivered (%d)", got);
	}
	else
		CHECK(0, "first fd delivered");
	CHECK(count_open(before, 10) == 1, "[darling] fds that did not fit are closed (%d open)", count_open(before, 10));
	close_range_(before, 10);

	// No room for a header at all.
	CHECK(send_rights(sv[0], fds, 3) == 1, "send 3 fds for an 8-byte buffer");
	before = lowest_free_fd();
	r = recv_into(sv[1], buf, 8, &m);
	CHECK(r == 1 && (m.msg_flags & MSG_CTRUNC), "8-byte buffer: MSG_CTRUNC (flags 0x%x)", m.msg_flags);
	CHECK(count_open(before, 10) == 0, "[darling] no fd left open (%d open)", count_open(before, 10));
	close_range_(before, 10);

	// Malformed control buffers: all EINVAL on macOS for AF_UNIX.
	{
		char cb[64] __attribute__((aligned(8)));
		struct iovec iov = { "x", 1 };
		struct msghdr sm;
		struct cmsghdr* sc = (struct cmsghdr*) cb;
		struct { const char* name; socklen_t controllen; unsigned len; int type; } cases[] = {
			{ "cmsg_len > msg_controllen", CMSG_SPACE(sizeof(int)), CMSG_SPACE(sizeof(int)) + 8, SCM_RIGHTS },
			{ "cmsg_len huge", CMSG_SPACE(sizeof(int)), 0xfffffff0u, SCM_RIGHTS },
			{ "cmsg_len < header", CMSG_SPACE(sizeof(int)), 4, SCM_RIGHTS },
			{ "msg_controllen < header", 4, CMSG_LEN(sizeof(int)), SCM_RIGHTS },
			{ "msg_controllen 0 with a buffer", 0, CMSG_LEN(sizeof(int)), SCM_RIGHTS },
			{ "4 trailing bytes", CMSG_SPACE(sizeof(int)) + 4, CMSG_LEN(sizeof(int)), SCM_RIGHTS },
			{ "SCM_RIGHTS not a whole fd", CMSG_SPACE(sizeof(int)), CMSG_LEN(3), SCM_RIGHTS },
			{ "SCM_CREDS", CMSG_SPACE(sizeof(int)), CMSG_LEN(sizeof(int)), SCM_CREDS },
			{ "SCM_TIMESTAMP", CMSG_SPACE(sizeof(int)), CMSG_LEN(sizeof(int)), SCM_TIMESTAMP },
		};

		for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
		{
			memset(cb, 0, sizeof(cb));
			memset(&sm, 0, sizeof(sm));
			sm.msg_iov = &iov;
			sm.msg_iovlen = 1;
			sm.msg_control = cb;
			sm.msg_controllen = cases[i].controllen;
			sc->cmsg_len = cases[i].len;
			sc->cmsg_level = SOL_SOCKET;
			sc->cmsg_type = cases[i].type;
			memcpy(CMSG_DATA(sc), &fds[0], sizeof(int));
			errno = 0;
			r = sendmsg(sv[0], &sm, MSG_DONTWAIT);
			CHECK(r == -1 && errno == EINVAL, "%s: EINVAL (ret %zd errno %d)", cases[i].name, r, errno);
		}

		// No descriptors at all: accepted, delivered as an empty SCM_RIGHTS.
		memset(&sm, 0, sizeof(sm));
		sm.msg_iov = &iov;
		sm.msg_iovlen = 1;
		sm.msg_control = cb;
		sm.msg_controllen = CMSG_SPACE(0);
		sc->cmsg_len = CMSG_LEN(0);
		sc->cmsg_type = SCM_RIGHTS;
		r = sendmsg(sv[0], &sm, 0);
		CHECK(r == 1, "SCM_RIGHTS with no fds accepted");
		if (r == 1)
			recv_into(sv[1], buf, sizeof(buf), &m);

		// msg_namelen without msg_name on a connected socket: accepted.
		memset(&sm, 0, sizeof(sm));
		sm.msg_iov = &iov;
		sm.msg_iovlen = 1;
		sm.msg_namelen = 16;
		r = sendmsg(sv[0], &sm, 0);
		CHECK(r == 1, "msg_namelen without msg_name accepted");
		if (r == 1)
			recv_into(sv[1], NULL, 0, &m);
	}

	close(sv[0]);
	close(sv[1]);
	for (int i = 0; i < 6; i++)
		close(fds[i]);
}

static struct cmsghdr* find(struct msghdr* m, int level, int type)
{
	for (struct cmsghdr* c = CMSG_FIRSTHDR(m); c != NULL; c = CMSG_NXTHDR(m, c))
		if (c->cmsg_level == level && c->cmsg_type == type)
			return c;
	return NULL;
}

static void test_udp4(void)
{
	int a = socket(AF_INET, SOCK_DGRAM, 0), b = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in sin, from;
	socklen_t sl = sizeof(sin);
	char buf[256] __attribute__((aligned(8)));
	char cb[64] __attribute__((aligned(8)));
	struct iovec iov = { "z", 1 };
	struct msghdr m, sm;
	struct cmsghdr* c;
	struct timeval now;
	int one = 1, ttl = 33;
	ssize_t r;

	memset(&sin, 0, sizeof(sin));
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	if (bind(b, (struct sockaddr*) &sin, sizeof(sin)) != 0 || getsockname(b, (struct sockaddr*) &sin, &sl) != 0)
	{
		CHECK(0, "UDP bind: %s", strerror(errno));
		return;
	}

	CHECK(setsockopt(b, SOL_SOCKET, SO_TIMESTAMP, &one, sizeof(one)) == 0, "SO_TIMESTAMP");
	CHECK(setsockopt(b, IPPROTO_IP, IP_RECVTTL, &one, sizeof(one)) == 0, "IP_RECVTTL");
	CHECK(setsockopt(b, IPPROTO_IP, IP_RECVPKTINFO, &one, sizeof(one)) == 0, "IP_RECVPKTINFO");
	CHECK(setsockopt(b, IPPROTO_IP, IP_RECVTOS, &one, sizeof(one)) == 0, "IP_RECVTOS");
	setsockopt(a, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

	// Two control messages on send: IP_TOS and IP_PKTINFO.
	memset(cb, 0, sizeof(cb));
	memset(&sm, 0, sizeof(sm));
	sm.msg_name = &sin;
	sm.msg_namelen = sizeof(sin);
	sm.msg_iov = &iov;
	sm.msg_iovlen = 1;
	sm.msg_control = cb;
	sm.msg_controllen = CMSG_SPACE(sizeof(int)) + CMSG_SPACE(sizeof(struct in_pktinfo));
	c = CMSG_FIRSTHDR(&sm);
	c->cmsg_level = IPPROTO_IP;
	c->cmsg_type = IP_TOS;
	c->cmsg_len = CMSG_LEN(sizeof(int));
	*(int*) CMSG_DATA(c) = 0x20;
	c = CMSG_NXTHDR(&sm, c);
	c->cmsg_level = IPPROTO_IP;
	c->cmsg_type = IP_PKTINFO;
	c->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
	CHECK(sendmsg(a, &sm, 0) == 1, "UDP send with IP_TOS + IP_PKTINFO");

	memset(&from, 0, sizeof(from));
	r = recv_into(b, buf, sizeof(buf), &m);
	gettimeofday(&now, NULL);
	CHECK(r == 1 && !(m.msg_flags & MSG_CTRUNC), "UDP recv (flags 0x%x)", m.msg_flags);

	c = find(&m, SOL_SOCKET, SCM_TIMESTAMP);
	CHECK(c != NULL && c->cmsg_len == CMSG_LEN(sizeof(struct timeval)), "SCM_TIMESTAMP, len %u", c ? c->cmsg_len : 0);
	if (c != NULL)
	{
		struct timeval tv;
		memcpy(&tv, CMSG_DATA(c), sizeof(tv));
		CHECK(tv.tv_sec >= now.tv_sec - 5 && tv.tv_sec <= now.tv_sec && tv.tv_usec >= 0 && tv.tv_usec < 1000000,
			"timestamp %ld.%06d close to now %ld", (long) tv.tv_sec, (int) tv.tv_usec, (long) now.tv_sec);
	}
	c = find(&m, IPPROTO_IP, IP_RECVTTL);
	CHECK(c != NULL && c->cmsg_len == CMSG_LEN(1) && *CMSG_DATA(c) == 33,
		"IP_RECVTTL: one byte, 33 (len %u, value %d)", c ? c->cmsg_len : 0, c ? *CMSG_DATA(c) : -1);
	c = find(&m, IPPROTO_IP, IP_RECVPKTINFO);
	CHECK(c != NULL && c->cmsg_len == CMSG_LEN(sizeof(struct in_pktinfo))
			&& ((struct in_pktinfo*) CMSG_DATA(c))->ipi_addr.s_addr == htonl(INADDR_LOOPBACK),
		"IP_RECVPKTINFO: in_pktinfo with 127.0.0.1");
	c = find(&m, IPPROTO_IP, IP_RECVTOS);
	CHECK(c != NULL && c->cmsg_len == CMSG_LEN(1) && *CMSG_DATA(c) == 0x20,
		"IP_RECVTOS: one byte, 0x20 from the sent IP_TOS (len %u, value 0x%x)", c ? c->cmsg_len : 0, c ? *CMSG_DATA(c) : 0);

	// Source address through msg_name.
	sendto(a, "y", 1, 0, (struct sockaddr*) &sin, sizeof(sin));
	{
		static char data[8];
		struct iovec riov = { data, sizeof(data) };
		memset(&m, 0, sizeof(m));
		m.msg_name = &from;
		m.msg_namelen = sizeof(from);
		m.msg_iov = &riov;
		m.msg_iovlen = 1;
		r = recvmsg(b, &m, 0);
		CHECK(r == 1 && m.msg_namelen == sizeof(from) && from.sin_family == AF_INET && from.sin_len == sizeof(from)
				&& from.sin_addr.s_addr == htonl(INADDR_LOOPBACK),
			"msg_name: AF_INET, sin_len %d, namelen %u", from.sin_len, (unsigned) m.msg_namelen);
		CHECK(m.msg_controllen == 0, "no control buffer: msg_controllen 0");
	}

	// Room for the timestamp and 4 more bytes: the first message only.
	sendto(a, "y", 1, 0, (struct sockaddr*) &sin, sizeof(sin));
	r = recv_into(b, buf, CMSG_SPACE(sizeof(struct timeval)) + 4, &m);
	c = CMSG_FIRSTHDR(&m);
	// macOS reports the whole buffer as used (it copies the start of the
	// message that did not fit); Darling reports the complete messages only.
	CHECK(r == 1 && (m.msg_flags & MSG_CTRUNC) && c != NULL && c->cmsg_type == SCM_TIMESTAMP
			&& m.msg_controllen >= CMSG_LEN(sizeof(struct timeval))
			&& m.msg_controllen <= CMSG_SPACE(sizeof(struct timeval)) + 4,
		"short buffer: timestamp first, MSG_CTRUNC, msg_controllen %u", (unsigned) m.msg_controllen);
	if (c != NULL)
	{
		c = CMSG_NXTHDR(&m, c);
		CHECK(c == NULL, "short buffer: no second message");
	}

	close(a);
	close(b);
}

static void test_udp6(void)
{
	int a = socket(AF_INET6, SOCK_DGRAM, 0), b = socket(AF_INET6, SOCK_DGRAM, 0);
	struct sockaddr_in6 sin;
	socklen_t sl = sizeof(sin);
	char buf[256] __attribute__((aligned(8)));
	char cb[32] __attribute__((aligned(8)));
	struct iovec iov = { "z", 1 };
	struct msghdr m, sm;
	struct cmsghdr* c;
	int one = 1;

	memset(&sin, 0, sizeof(sin));
	sin.sin6_len = sizeof(sin);
	sin.sin6_family = AF_INET6;
	sin.sin6_addr = in6addr_loopback;
	if (a < 0 || b < 0 || bind(b, (struct sockaddr*) &sin, sizeof(sin)) != 0
			|| getsockname(b, (struct sockaddr*) &sin, &sl) != 0)
	{
		printf("SKIP: no IPv6 loopback (%s)\n", strerror(errno));
		return;
	}

	CHECK(setsockopt(b, IPPROTO_IPV6, IPV6_RECVPKTINFO, &one, sizeof(one)) == 0, "IPV6_RECVPKTINFO");
	CHECK(setsockopt(b, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &one, sizeof(one)) == 0, "IPV6_RECVHOPLIMIT");
	CHECK(setsockopt(b, IPPROTO_IPV6, IPV6_RECVTCLASS, &one, sizeof(one)) == 0, "IPV6_RECVTCLASS");

	memset(cb, 0, sizeof(cb));
	memset(&sm, 0, sizeof(sm));
	sm.msg_name = &sin;
	sm.msg_namelen = sizeof(sin);
	sm.msg_iov = &iov;
	sm.msg_iovlen = 1;
	sm.msg_control = cb;
	sm.msg_controllen = CMSG_SPACE(sizeof(int));
	c = CMSG_FIRSTHDR(&sm);
	c->cmsg_level = IPPROTO_IPV6;
	c->cmsg_type = IPV6_HOPLIMIT;
	c->cmsg_len = CMSG_LEN(sizeof(int));
	*(int*) CMSG_DATA(c) = 9;
	CHECK(sendmsg(a, &sm, 0) == 1, "UDP6 send with IPV6_HOPLIMIT 9");

	recv_into(b, buf, sizeof(buf), &m);
	c = find(&m, IPPROTO_IPV6, IPV6_PKTINFO);
	CHECK(c != NULL && c->cmsg_len == CMSG_LEN(sizeof(struct in6_pktinfo))
			&& IN6_IS_ADDR_LOOPBACK(&((struct in6_pktinfo*) CMSG_DATA(c))->ipi6_addr),
		"IPV6_PKTINFO: in6_pktinfo with ::1");
	c = find(&m, IPPROTO_IPV6, IPV6_HOPLIMIT);
	CHECK(c != NULL && c->cmsg_len == CMSG_LEN(sizeof(int)) && *(int*) CMSG_DATA(c) == 9,
		"IPV6_HOPLIMIT: int 9 (value %d)", c ? *(int*) CMSG_DATA(c) : -1);
	c = find(&m, IPPROTO_IPV6, IPV6_TCLASS);
	CHECK(c != NULL && c->cmsg_len == CMSG_LEN(sizeof(int)), "IPV6_TCLASS: int");

	close(a);
	close(b);
}

int main(void)
{
	test_rights();
	test_udp4();
	test_udp6();

	printf(failures ? "FAILED: %d\n" : "ALL PASSED\n", failures);
	return failures != 0;
}
