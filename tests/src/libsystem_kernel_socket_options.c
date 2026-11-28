/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Socket options that XNU answers itself or numbers differently from Linux. Expected results from <sys/socket.h> and
 * <netinet/in.h> and, where those are silent, as observed on macOS 26.7.3 (black-box).
 *  - SO_TRAFFIC_CLASS takes 0, 100, ..., 900 or 10000 + a network service type; SO_NET_SERVICE_TYPE takes 0..8 and
 *    sets the traffic class that type maps to; other values are EINVAL.
 *  - SO_RECV_ANYIF is a flag; IP_RECVIF and IP_RECVDSTADDR make recvmsg() return an IP_RECVDSTADDR message (the
 *    destination address) and an IP_RECVIF message (a struct sockaddr_dl carrying the interface index).
 *  - IP_RECVDSTADDR (7) sits before IP_RETOPTS (8) and the multicast options (9..13), which take their own values.
 *  - IP_MULTICAST_IFINDEX and the MCAST_* group options of <netinet/in.h> are accepted.
 * Before the change the traffic class and RECV_ANYIF options were EINVAL and every IP option from IP_RETOPTS on was
 * mapped one Linux option too low.
 */
#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef SO_TRAFFIC_CLASS
#define SO_TRAFFIC_CLASS 0x1086
#define SO_RECV_TRAFFIC_CLASS 0x1087
#endif
#ifndef SO_NET_SERVICE_TYPE
#define SO_NET_SERVICE_TYPE 0x1116
#endif
#ifndef SO_RECV_ANYIF
#define SO_RECV_ANYIF 0x1104
#endif

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); failures++; } } while (0)

static int getint(int s, int level, int name)
{
	int v = -12345;
	socklen_t len = sizeof v;
	if (getsockopt(s, level, name, &v, &len) != 0)
		return -99999;
	return v;
}

static int setint(int s, int level, int name, int v)
{
	return setsockopt(s, level, name, &v, sizeof v);
}

int main(void)
{
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	CHECK(s >= 0);

	// SO_TRAFFIC_CLASS: exact classes
	for (int tc = 0; tc <= 900; tc += 100)
	{
		CHECK(setint(s, SOL_SOCKET, SO_TRAFFIC_CLASS, tc) == 0);
		CHECK(getint(s, SOL_SOCKET, SO_TRAFFIC_CLASS) == tc);
	}
	int bad[] = { 1000, 250, -1, 10009 };
	for (unsigned i = 0; i < sizeof bad / sizeof *bad; i++)
	{
		CHECK(setint(s, SOL_SOCKET, SO_TRAFFIC_CLASS, bad[i]) == -1 && errno == EINVAL);
	}
	// network service types: the class each maps to
	static const int cls[9] = { 0, 200, 700, 700, 800, 600, 500, 400, 300 };
	for (int svc = 0; svc <= 8; svc++)
	{
		CHECK(setint(s, SOL_SOCKET, SO_NET_SERVICE_TYPE, svc) == 0);
		CHECK(getint(s, SOL_SOCKET, SO_NET_SERVICE_TYPE) == svc);
		CHECK(getint(s, SOL_SOCKET, SO_TRAFFIC_CLASS) == cls[svc]);
		CHECK(setint(s, SOL_SOCKET, SO_TRAFFIC_CLASS, 10000 + svc) == 0);
		CHECK(getint(s, SOL_SOCKET, SO_TRAFFIC_CLASS) == cls[svc]);
	}
	CHECK(setint(s, SOL_SOCKET, SO_NET_SERVICE_TYPE, 9) == -1 && errno == EINVAL);
	// QUIRK: SO_RECV_TRAFFIC_CLASS reads back as 0x4000 when set (undocumented; macOS 26.7.3)
	CHECK(getint(s, SOL_SOCKET, SO_RECV_TRAFFIC_CLASS) == 0);
	CHECK(setint(s, SOL_SOCKET, SO_RECV_TRAFFIC_CLASS, 1) == 0);
	CHECK(getint(s, SOL_SOCKET, SO_RECV_TRAFFIC_CLASS) == 0x4000);
	CHECK(setint(s, SOL_SOCKET, SO_RECV_TRAFFIC_CLASS, 0) == 0);
	CHECK(getint(s, SOL_SOCKET, SO_RECV_TRAFFIC_CLASS) == 0);

	// SO_RECV_ANYIF
	CHECK(getint(s, SOL_SOCKET, SO_RECV_ANYIF) == 0);
	CHECK(setint(s, SOL_SOCKET, SO_RECV_ANYIF, 1) == 0);
	CHECK(getint(s, SOL_SOCKET, SO_RECV_ANYIF) == 1);

	// IP option numbering
	CHECK(IP_RECVDSTADDR == 7 && IP_RETOPTS == 8 && IP_MULTICAST_IF == 9 && IP_MULTICAST_TTL == 10);
	unsigned char ttl = 7, lp = 0;
	CHECK(setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, 1) == 0);
	ttl = 0;
	socklen_t one = 1;
	CHECK(getsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, &one) == 0 && ttl == 7);
	CHECK(setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &lp, 1) == 0);
	lp = 9;
	one = 1;
	CHECK(getsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &lp, &one) == 0 && lp == 0);
	CHECK(setint(s, IPPROTO_IP, IP_RECVDSTADDR, 1) == 0);
	CHECK(getint(s, IPPROTO_IP, IP_RECVDSTADDR) == 1);

	// IP_MULTICAST_IFINDEX and the protocol-independent group options are accepted
	CHECK(setint(s, IPPROTO_IP, IP_MULTICAST_IFINDEX, 1) == 0);
	{
		struct { uint32_t gr_interface; struct sockaddr_storage gr_group; } __attribute__((packed, aligned(4))) gr;
		memset(&gr, 0, sizeof gr);
		gr.gr_interface = 1;
		struct sockaddr_in* g = (struct sockaddr_in*) &gr.gr_group;
		g->sin_len = sizeof *g;
		g->sin_family = AF_INET;
		g->sin_addr.s_addr = inet_addr("239.255.77.1");
		int jr = setsockopt(s, IPPROTO_IP, MCAST_JOIN_GROUP, &gr, 4 + sizeof(struct sockaddr_storage));
		CHECK(jr == 0);
		CHECK(setsockopt(s, IPPROTO_IP, MCAST_LEAVE_GROUP, &gr, 4 + sizeof(struct sockaddr_storage)) == 0);
	}

	// recvmsg: IP_RECVDSTADDR and IP_RECVIF messages
	int rs = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in ra;
	memset(&ra, 0, sizeof ra);
	ra.sin_len = sizeof ra;
	ra.sin_family = AF_INET;
	ra.sin_addr.s_addr = htonl(INADDR_ANY);
	CHECK(bind(rs, (struct sockaddr*) &ra, sizeof ra) == 0);
	socklen_t rl = sizeof ra;
	CHECK(getsockname(rs, (struct sockaddr*) &ra, &rl) == 0);
	CHECK(setint(rs, IPPROTO_IP, IP_RECVDSTADDR, 1) == 0);
	CHECK(setint(rs, IPPROTO_IP, IP_RECVIF, 1) == 0);
	CHECK(getint(rs, IPPROTO_IP, IP_RECVIF) == 1);
	struct sockaddr_in to = ra;
	to.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	CHECK(sendto(s, "hi", 2, 0, (struct sockaddr*) &to, sizeof to) == 2);
	usleep(100000);
	char data[16], cbuf[256];
	struct iovec iov = { data, sizeof data };
	struct msghdr mh;
	memset(&mh, 0, sizeof mh);
	mh.msg_iov = &iov;
	mh.msg_iovlen = 1;
	mh.msg_control = cbuf;
	mh.msg_controllen = sizeof cbuf;
	CHECK(recvmsg(rs, &mh, MSG_DONTWAIT) == 2);
	int saw_dst = 0, saw_if = 0;
	for (struct cmsghdr* c = CMSG_FIRSTHDR(&mh); c; c = CMSG_NXTHDR(&mh, c))
	{
		CHECK(c->cmsg_level == IPPROTO_IP);
		if (c->cmsg_type == IP_RECVDSTADDR)
		{
			saw_dst++;
			CHECK(c->cmsg_len == CMSG_LEN(4));
			CHECK(memcmp(CMSG_DATA(c), &to.sin_addr, 4) == 0);
		}
		else if (c->cmsg_type == IP_RECVIF)
		{
			saw_if++;
			unsigned char* d = CMSG_DATA(c);
			CHECK(c->cmsg_len == CMSG_LEN(20));
			CHECK(d[0] == 20 && d[1] == 18); // sdl_len, AF_LINK
			CHECK((d[2] | d[3] << 8) >= 1); // sdl_index
		}
		else
			CHECK(!"unexpected control message");
	}
	CHECK(saw_dst == 1 && saw_if == 1);

	close(rs);
	close(s);
	printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
	return failures != 0;
}
