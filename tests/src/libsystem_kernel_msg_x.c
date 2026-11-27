/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * recvmsg_x() and sendmsg_x() (syscalls 480 and 481): batched datagram receive and send.
 * Expected results observed on macOS 10.14.4 (i386, x86_64) and macOS 26.7.3 (x86_64):
 *  - sendmsg_x returns how many datagrams it sent and leaves msg_datalen alone;
 *  - recvmsg_x returns how many it received (the first may block, the rest only come from the queue),
 *    each msg_datalen is the datagram's length;
 *  - a datagram that does not fit ends the batch (macOS 10.14; macOS 26 carries on, and this follows 10.14);
 *  - a count of 0 to recvmsg_x is EINVAL, an empty queue with MSG_DONTWAIT is EAGAIN.
 */
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

struct msghdr_x {
	void* msg_name;
	socklen_t msg_namelen;
	struct iovec* msg_iov;
	int msg_iovlen;
	void* msg_control;
	socklen_t msg_controllen;
	int msg_flags;
	size_t msg_datalen;
};

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); failures++; } } while (0)

int main(void)
{
	long (*recvmsg_x)(int, struct msghdr_x*, unsigned, int) = dlsym(RTLD_DEFAULT, "recvmsg_x");
	long (*sendmsg_x)(int, struct msghdr_x*, unsigned, int) = dlsym(RTLD_DEFAULT, "sendmsg_x");
	int sv[2];
	struct iovec iov[4];
	struct msghdr_x m[4];
	char bufs[4][16];
	const char* texts[3] = { "aaaa", "bbbbbb", "cc" };
	long r;

	if (!recvmsg_x || !sendmsg_x) {
		puts("FAIL: recvmsg_x / sendmsg_x not found");
		return 1;
	}
	CHECK(socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) == 0);

	for (int i = 0; i < 3; i++) {
		iov[i].iov_base = (void*) texts[i];
		iov[i].iov_len = strlen(texts[i]);
		memset(&m[i], 0, sizeof(m[i]));
		m[i].msg_iov = &iov[i];
		m[i].msg_iovlen = 1;
		m[i].msg_datalen = 0xdead;
	}
	errno = 0;
	r = sendmsg_x(sv[0], m, 3, 0);
	CHECK(r == 3);
	for (int i = 0; i < 3; i++)
		CHECK(m[i].msg_datalen == 0xdead);

	for (int i = 0; i < 4; i++) {
		iov[i].iov_base = bufs[i];
		iov[i].iov_len = sizeof(bufs[i]);
		memset(&m[i], 0, sizeof(m[i]));
		m[i].msg_iov = &iov[i];
		m[i].msg_iovlen = 1;
	}
	errno = 0;
	r = recvmsg_x(sv[1], m, 4, MSG_DONTWAIT);
	CHECK(r == 3);
	CHECK(m[0].msg_datalen == 4 && memcmp(bufs[0], "aaaa", 4) == 0);
	CHECK(m[1].msg_datalen == 6 && memcmp(bufs[1], "bbbbbb", 6) == 0);
	CHECK(m[2].msg_datalen == 2 && memcmp(bufs[2], "cc", 2) == 0);

	errno = 0;
	CHECK(recvmsg_x(sv[1], m, 4, MSG_DONTWAIT) == -1 && errno == EAGAIN);
	errno = 0;
	CHECK(recvmsg_x(sv[1], m, 0, MSG_DONTWAIT) == -1 && errno == EINVAL);

	/* a datagram longer than its buffer ends the batch */
	iov[0].iov_base = (void*) "0123456789";
	iov[0].iov_len = 10;
	memset(&m[0], 0, sizeof(m[0]));
	m[0].msg_iov = &iov[0];
	m[0].msg_iovlen = 1;
	iov[1].iov_base = (void*) "zz";
	iov[1].iov_len = 2;
	memset(&m[1], 0, sizeof(m[1]));
	m[1].msg_iov = &iov[1];
	m[1].msg_iovlen = 1;
	CHECK(sendmsg_x(sv[0], m, 2, 0) == 2);

	char small[4];
	struct iovec si[2] = { { small, 4 }, { small, 4 } };
	struct msghdr_x rm[2];
	memset(rm, 0, sizeof(rm));
	rm[0].msg_iov = &si[0];
	rm[0].msg_iovlen = 1;
	rm[1].msg_iov = &si[1];
	rm[1].msg_iovlen = 1;
	errno = 0;
	r = recvmsg_x(sv[1], rm, 2, MSG_DONTWAIT);
	CHECK(r == 1);
	CHECK(rm[0].msg_datalen == 4);
	/* the second datagram stays queued */
	char rest[8];
	CHECK(recv(sv[1], rest, sizeof(rest), MSG_DONTWAIT) == 2);

	if (failures == 0)
		puts("PASS");
	return failures != 0;
}
