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
 * libproc descriptor flavours: proc_pidinfo(PROC_PIDLISTFDS) and
 * proc_pidfdinfo(PROC_PIDFDSOCKETINFO / PROC_PIDFDPIPEINFO /
 * PROC_PIDFDVNODEPATHINFO), including the size query, short buffers and
 * the errors for bad pids and descriptors, and inspecting another process.
 * Expected values were observed on macOS 26.7.3.
 *
 * Prints PASS/FAIL per check; the exit status is the number of failures.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libproc.h>
#include <sys/proc_info.h>

static int failures;
#define CHECK(name, cond) do { int ok_ = !!(cond); \
	printf("%s: %s\n", ok_ ? "PASS" : "FAIL", name); if (!ok_) failures++; } while (0)

static int fd_type(const struct proc_fdinfo *l, int n, int fd)
{
	for (int i = 0; i < n; i++)
		if (l[i].proc_fd == fd)
			return (int) l[i].proc_fdtype;
	return -1;
}

/* Darling reports host paths below /Volumes/SystemRoot */
static int ends_with(const char *s, const char *suffix)
{
	size_t n = strlen(s), m = strlen(suffix);
	return n >= m && strcmp(s + n - m, suffix) == 0;
}

static int call_fdinfo(int pid, int fd, int flavor, void *buf, int size)
{
	errno = 0;
	return proc_pidfdinfo(pid, fd, flavor, buf, size);
}

static int tcp_info(int pid, int fd, struct socket_fdinfo *si)
{
	memset(si, 0, sizeof *si);
	return call_fdinfo(pid, fd, PROC_PIDFDSOCKETINFO, si, sizeof *si) == (int) sizeof *si;
}

/* Local/foreign address and port as presented for an IPv4 socket */
static int v4_endpoints(const struct in_sockinfo *i, in_addr_t laddr, int lport, in_addr_t faddr, int fport)
{
	return (i->insi_vflag & INI_IPV4) &&
	    i->insi_laddr.ina_46.i46a_addr4.s_addr == laddr && ntohs(i->insi_lport) == lport &&
	    i->insi_faddr.ina_46.i46a_addr4.s_addr == faddr && ntohs(i->insi_fport) == fport;
}

int main(void)
{
	char big[512];
	int pid = getpid();
	in_addr_t lo = htonl(INADDR_LOOPBACK);

	/* A loopback TCP pair, a UDP socket, an IPv6 listener, a unix pair,
	 * a pipe, a kqueue and an open vnode */
	int lfd = socket(AF_INET, SOCK_STREAM, 0), cfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in a = { .sin_len = sizeof a, .sin_family = AF_INET };
	a.sin_addr.s_addr = lo;
	socklen_t al = sizeof a;
	if (bind(lfd, (void *) &a, sizeof a) || getsockname(lfd, (void *) &a, &al) || listen(lfd, 4) ||
	    connect(cfd, (void *) &a, sizeof a)) {
		printf("FAIL: loopback setup (errno %d)\n", errno);
		return 1;
	}
	int sfd = accept(lfd, NULL, NULL);
	int lport = ntohs(a.sin_port);
	struct sockaddr_in ca;
	al = sizeof ca;
	getsockname(cfd, (void *) &ca, &al);
	int cport = ntohs(ca.sin_port);

	int ufd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in ua = { .sin_len = sizeof ua, .sin_family = AF_INET };
	ua.sin_addr.s_addr = lo;
	bind(ufd, (void *) &ua, sizeof ua);
	al = sizeof ua;
	getsockname(ufd, (void *) &ua, &al);

	int s6 = socket(AF_INET6, SOCK_STREAM, 0);
	struct sockaddr_in6 a6 = { .sin6_len = sizeof a6, .sin6_family = AF_INET6, .sin6_addr = IN6ADDR_LOOPBACK_INIT };
	int have6 = s6 >= 0 && bind(s6, (void *) &a6, sizeof a6) == 0 && listen(s6, 1) == 0;
	al = sizeof a6;
	if (have6)
		getsockname(s6, (void *) &a6, &al);

	int sv[2], pp[2];
	socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
	pipe(pp);
	int kq = kqueue();
	int vf = open("/dev/null", O_RDONLY);

	/* The listing */
	int need = proc_pidinfo(pid, PROC_PIDLISTFDS, 0, NULL, 0);
	CHECK("size query returns a positive multiple of the entry size",
	    need > 0 && need % (int) sizeof(struct proc_fdinfo) == 0);
	errno = 0;
	CHECK("size query ignores the buffer size when the buffer is NULL",
	    proc_pidinfo(pid, PROC_PIDLISTFDS, 0, NULL, 4) == need);

	int cap = (need > 0 ? need : 8) + 256;
	struct proc_fdinfo *l = malloc(cap);
	int got = proc_pidinfo(pid, PROC_PIDLISTFDS, 0, l, cap);
	int n = got / (int) sizeof *l;
	CHECK("fill returns whole entries and no more than the size query", got > 0 && got % (int) sizeof *l == 0 && got <= need);
	int sorted = 1;
	for (int i = 1; i < n; i++)
		if (l[i].proc_fd <= l[i - 1].proc_fd)
			sorted = 0;
	CHECK("entries are in ascending descriptor order", sorted);
	CHECK("TCP sockets are PROX_FDTYPE_SOCKET",
	    fd_type(l, n, lfd) == PROX_FDTYPE_SOCKET && fd_type(l, n, cfd) == PROX_FDTYPE_SOCKET &&
	    fd_type(l, n, sfd) == PROX_FDTYPE_SOCKET);
	CHECK("UDP and unix sockets are PROX_FDTYPE_SOCKET",
	    fd_type(l, n, ufd) == PROX_FDTYPE_SOCKET && fd_type(l, n, sv[0]) == PROX_FDTYPE_SOCKET &&
	    fd_type(l, n, sv[1]) == PROX_FDTYPE_SOCKET);
	CHECK("pipe ends are PROX_FDTYPE_PIPE",
	    fd_type(l, n, pp[0]) == PROX_FDTYPE_PIPE && fd_type(l, n, pp[1]) == PROX_FDTYPE_PIPE);
	CHECK("a kqueue is PROX_FDTYPE_KQUEUE", fd_type(l, n, kq) == PROX_FDTYPE_KQUEUE);
	CHECK("an open file is PROX_FDTYPE_VNODE", fd_type(l, n, vf) == PROX_FDTYPE_VNODE);
	CHECK("a closed descriptor is absent", fd_type(l, n, 900) == -1);

	/* Buffer sizes: below one entry fails, otherwise whole entries are copied */
	errno = 0;
	CHECK("zero-size buffer: 0 with ENOMEM", proc_pidinfo(pid, PROC_PIDLISTFDS, 0, l, 0) == 0 && errno == ENOMEM);
	errno = 0;
	CHECK("buffer below one entry: 0 with ENOMEM",
	    proc_pidinfo(pid, PROC_PIDLISTFDS, 0, l, sizeof *l - 1) == 0 && errno == ENOMEM);
	CHECK("buffer of one entry: one entry", proc_pidinfo(pid, PROC_PIDLISTFDS, 0, l, sizeof *l) == (int) sizeof *l);
	CHECK("buffer of two and a half entries: two entries",
	    proc_pidinfo(pid, PROC_PIDLISTFDS, 0, l, 2 * sizeof *l + 4) == (int) (2 * sizeof *l));

	/* Errors for the process */
	errno = 0;
	CHECK("nonexistent pid: ESRCH", proc_pidinfo(999999, PROC_PIDLISTFDS, 0, l, cap) == 0 && errno == ESRCH);
	errno = 0;
	CHECK("nonexistent pid, size query: ESRCH", proc_pidinfo(999999, PROC_PIDLISTFDS, 0, NULL, 0) == 0 && errno == ESRCH);
	errno = 0;
	CHECK("pid -1: ESRCH", proc_pidinfo(-1, PROC_PIDLISTFDS, 0, l, cap) == 0 && errno == ESRCH);
	errno = 0;
	CHECK("pid 0: EPERM", proc_pidinfo(0, PROC_PIDLISTFDS, 0, NULL, 0) == 0 && errno == EPERM);
	free(l);

	/* Socket info: TCP listener */
	struct socket_fdinfo si;
	CHECK("listener: socket info returned", tcp_info(pid, lfd, &si));
	CHECK("listener: fi_type is socket", si.pfi.fi_type == PROX_FDTYPE_SOCKET);
	CHECK("listener: family, type, protocol",
	    si.psi.soi_family == AF_INET && si.psi.soi_type == SOCK_STREAM && si.psi.soi_protocol == IPPROTO_TCP);
	CHECK("listener: kind is SOCKINFO_TCP", si.psi.soi_kind == SOCKINFO_TCP);
	CHECK("listener: state is LISTEN", si.psi.soi_proto.pri_tcp.tcpsi_state == TSI_S_LISTEN);
	CHECK("listener: local endpoint 127.0.0.1:port, no foreign endpoint",
	    v4_endpoints(&si.psi.soi_proto.pri_tcp.tcpsi_ini, lo, lport, 0, 0));
	CHECK("listener: socket handle is nonzero", si.psi.soi_so != 0);

	/* Both ends of the connected pair: the endpoints mirror each other */
	struct socket_fdinfo cs, ss;
	CHECK("client: socket info returned", tcp_info(pid, cfd, &cs));
	CHECK("accepted: socket info returned", tcp_info(pid, sfd, &ss));
	CHECK("client: ESTABLISHED, connected to the listener's port",
	    cs.psi.soi_kind == SOCKINFO_TCP && cs.psi.soi_proto.pri_tcp.tcpsi_state == TSI_S_ESTABLISHED &&
	    v4_endpoints(&cs.psi.soi_proto.pri_tcp.tcpsi_ini, lo, cport, lo, lport));
	CHECK("accepted: ESTABLISHED, the owner of the peer's port is found from the other end",
	    ss.psi.soi_kind == SOCKINFO_TCP && ss.psi.soi_proto.pri_tcp.tcpsi_state == TSI_S_ESTABLISHED &&
	    v4_endpoints(&ss.psi.soi_proto.pri_tcp.tcpsi_ini, lo, lport, lo, cport));
	CHECK("connected sockets have SOI_S_ISCONNECTED set",
	    (cs.psi.soi_state & SOI_S_ISCONNECTED) && (ss.psi.soi_state & SOI_S_ISCONNECTED));
	CHECK("the two ends have different socket handles", cs.psi.soi_so != ss.psi.soi_so);

	/* UDP over IPv4 */
	struct socket_fdinfo us;
	CHECK("UDP: socket info returned", tcp_info(pid, ufd, &us));
	CHECK("UDP: family, type, protocol",
	    us.psi.soi_family == AF_INET && us.psi.soi_type == SOCK_DGRAM && us.psi.soi_protocol == IPPROTO_UDP);
	CHECK("UDP: kind is SOCKINFO_IN with the bound endpoint",
	    us.psi.soi_kind == SOCKINFO_IN && v4_endpoints(&us.psi.soi_proto.pri_in, lo, ntohs(ua.sin_port), 0, 0));

	/* TCP over IPv6 */
	if (have6) {
		struct socket_fdinfo s6i;
		CHECK("IPv6 listener: socket info returned", tcp_info(pid, s6, &s6i));
		const struct in_sockinfo *i6 = &s6i.psi.soi_proto.pri_tcp.tcpsi_ini;
		CHECK("IPv6 listener: family, type, protocol",
		    s6i.psi.soi_family == AF_INET6 && s6i.psi.soi_type == SOCK_STREAM && s6i.psi.soi_protocol == IPPROTO_TCP);
		CHECK("IPv6 listener: LISTEN on [::1]:port",
		    s6i.psi.soi_kind == SOCKINFO_TCP && s6i.psi.soi_proto.pri_tcp.tcpsi_state == TSI_S_LISTEN &&
		    (i6->insi_vflag & INI_IPV6) && memcmp(&i6->insi_laddr.ina_6, &in6addr_loopback, 16) == 0 &&
		    ntohs(i6->insi_lport) == ntohs(a6.sin6_port) && ntohs(i6->insi_fport) == 0);
	} else {
		printf("SKIP: IPv6 loopback unavailable\n");
	}

	/* Unix socket */
	struct socket_fdinfo xs;
	CHECK("unix socket: info returned", tcp_info(pid, sv[0], &xs));
	CHECK("unix socket: family, type, kind",
	    xs.psi.soi_family == AF_UNIX && xs.psi.soi_type == SOCK_STREAM && xs.psi.soi_kind == SOCKINFO_UN);

	/* Another process: the child inspects the parent's sockets */
	pid_t child = fork();
	if (child == 0) {
		struct socket_fdinfo ps;
		int ok = tcp_info(getppid(), sfd, &ps) && ps.psi.soi_kind == SOCKINFO_TCP &&
		    v4_endpoints(&ps.psi.soi_proto.pri_tcp.tcpsi_ini, lo, lport, lo, cport);
		int need2 = proc_pidinfo(getppid(), PROC_PIDLISTFDS, 0, NULL, 0);
		_exit(ok && need2 > 0 ? 0 : 1);
	}
	int status = 0;
	waitpid(child, &status, 0);
	CHECK("a child reads the parent's descriptors and the peer's endpoints", WIFEXITED(status) && WEXITSTATUS(status) == 0);

	/* Errors of the descriptor flavours */
	struct socket_fdinfo tmp;
	CHECK("bad fd: EBADF", call_fdinfo(pid, 999, PROC_PIDFDSOCKETINFO, &tmp, sizeof tmp) == 0 && errno == EBADF);
	CHECK("negative fd: EBADF", call_fdinfo(pid, -1, PROC_PIDFDSOCKETINFO, &tmp, sizeof tmp) == 0 && errno == EBADF);
	CHECK("bad pid: ESRCH", call_fdinfo(999999, cfd, PROC_PIDFDSOCKETINFO, &tmp, sizeof tmp) == 0 && errno == ESRCH);
	CHECK("socket info of a pipe: ENOTSOCK",
	    call_fdinfo(pid, pp[0], PROC_PIDFDSOCKETINFO, &tmp, sizeof tmp) == 0 && errno == ENOTSOCK);
	CHECK("socket info of a file: ENOTSOCK",
	    call_fdinfo(pid, vf, PROC_PIDFDSOCKETINFO, &tmp, sizeof tmp) == 0 && errno == ENOTSOCK);
	CHECK("short buffer: ENOMEM",
	    call_fdinfo(pid, cfd, PROC_PIDFDSOCKETINFO, &tmp, sizeof tmp - 8) == 0 && errno == ENOMEM);
	CHECK("zero-size buffer: ENOMEM", call_fdinfo(pid, cfd, PROC_PIDFDSOCKETINFO, &tmp, 0) == 0 && errno == ENOMEM);
	CHECK("NULL buffer: EFAULT", call_fdinfo(pid, cfd, PROC_PIDFDSOCKETINFO, NULL, sizeof tmp) == 0 && errno == EFAULT);
	CHECK("larger buffer: structure size is returned",
	    call_fdinfo(pid, cfd, PROC_PIDFDSOCKETINFO, &tmp, sizeof tmp) == (int) sizeof tmp);

	/* Pipe info */
	struct pipe_fdinfo pr, pw;
	memset(&pr, 0, sizeof pr);
	memset(&pw, 0, sizeof pw);
	CHECK("pipe info of both ends returned",
	    call_fdinfo(pid, pp[0], PROC_PIDFDPIPEINFO, &pr, sizeof pr) == (int) sizeof pr &&
	    call_fdinfo(pid, pp[1], PROC_PIDFDPIPEINFO, &pw, sizeof pw) == (int) sizeof pw);
	CHECK("pipe info: fi_type is pipe, read end FREAD, write end FWRITE",
	    pr.pfi.fi_type == PROX_FDTYPE_PIPE && pr.pfi.fi_openflags == 1 && pw.pfi.fi_openflags == 2);
	CHECK("pipe info: each end names the other as its peer",
	    pr.pipeinfo.pipe_handle != 0 && pr.pipeinfo.pipe_peerhandle != 0 &&
	    pw.pipeinfo.pipe_handle == pr.pipeinfo.pipe_peerhandle && pw.pipeinfo.pipe_peerhandle == pr.pipeinfo.pipe_handle);
	CHECK("pipe info: the stat mode says FIFO", (pr.pipeinfo.pipe_stat.vst_mode & 0170000) == 0010000);
	CHECK("pipe info of a socket: EBADF", call_fdinfo(pid, cfd, PROC_PIDFDPIPEINFO, &pr, sizeof pr) == 0 && errno == EBADF);
	CHECK("pipe info of a file: EBADF", call_fdinfo(pid, vf, PROC_PIDFDPIPEINFO, &pr, sizeof pr) == 0 && errno == EBADF);
	CHECK("pipe info, short buffer: ENOMEM",
	    call_fdinfo(pid, pp[0], PROC_PIDFDPIPEINFO, &pr, sizeof pr - 1) == 0 && errno == ENOMEM);

	/* Vnode path info */
	struct vnode_fdinfowithpath vi;
	memset(&vi, 0, sizeof vi);
	CHECK("vnode path info of /dev/null",
	    call_fdinfo(pid, vf, PROC_PIDFDVNODEPATHINFO, &vi, sizeof vi) == (int) sizeof vi &&
	    vi.pfi.fi_type == PROX_FDTYPE_VNODE && ends_with(vi.pvip.vip_path, "/dev/null"));
	CHECK("vnode path info of a socket: EBADF",
	    call_fdinfo(pid, cfd, PROC_PIDFDVNODEPATHINFO, &vi, sizeof vi) == 0 && errno == EBADF);

	/* Kqueue info (PROC_PIDFDKQUEUEINFO) */
	struct kqueue_fdinfo ki;
	memset(&ki, 0xAA, sizeof ki);
	CHECK("kqueue info returns sizeof(struct kqueue_fdinfo)",
	    call_fdinfo(pid, kq, PROC_PIDFDKQUEUEINFO, &ki, sizeof ki) == (int) sizeof ki);
	CHECK("kqueue info: fi_type is kqueue, open for reading and writing",
	    ki.pfi.fi_type == PROX_FDTYPE_KQUEUE && ki.pfi.fi_openflags == 3);
	CHECK("kqueue info: kq_stat is a nameless FIFO-like object", (ki.kqueueinfo.kq_stat.vst_mode & 0170000) == 0010000);
	CHECK("kqueue info: kq_state has only the documented flags",
	    (ki.kqueueinfo.kq_state & ~(PROC_KQUEUE_SELECT | PROC_KQUEUE_SLEEP | PROC_KQUEUE_32 | PROC_KQUEUE_64 | PROC_KQUEUE_QOS)) == 0);
	struct kevent kev;
	EV_SET(&kev, pp[0], EVFILT_READ, EV_ADD, 0, 0, NULL);
	CHECK("register an event on the kqueue", kevent(kq, &kev, 1, NULL, 0, NULL) == 0);
	CHECK("kqueue info with a registered event",
	    call_fdinfo(pid, kq, PROC_PIDFDKQUEUEINFO, &ki, sizeof ki) == (int) sizeof ki && ki.pfi.fi_type == PROX_FDTYPE_KQUEUE);
	CHECK("kqueue info, bigger buffer: same size returned",
	    call_fdinfo(pid, kq, PROC_PIDFDKQUEUEINFO, big, sizeof big) == (int) sizeof ki);
	CHECK("kqueue info, short buffer: ENOMEM",
	    call_fdinfo(pid, kq, PROC_PIDFDKQUEUEINFO, &ki, sizeof ki - 1) == 0 && errno == ENOMEM);
	CHECK("kqueue info of a file: EBADF", call_fdinfo(pid, vf, PROC_PIDFDKQUEUEINFO, &ki, sizeof ki) == 0 && errno == EBADF);
	CHECK("kqueue info of a pipe: EBADF", call_fdinfo(pid, pp[0], PROC_PIDFDKQUEUEINFO, &ki, sizeof ki) == 0 && errno == EBADF);
	CHECK("kqueue info of a closed descriptor: EBADF", call_fdinfo(pid, 777, PROC_PIDFDKQUEUEINFO, &ki, sizeof ki) == 0 && errno == EBADF);
	CHECK("kqueue info of a nonexistent process: ESRCH", call_fdinfo(999999, kq, PROC_PIDFDKQUEUEINFO, &ki, sizeof ki) == 0 && errno == ESRCH);
	CHECK("kqueue info of a socket: EBADF", call_fdinfo(pid, cfd, PROC_PIDFDKQUEUEINFO, &ki, sizeof ki) == 0 && errno == EBADF);

	/* Unsupported flavors fail with EINVAL and write nothing to stderr */
	{
		int ep[2], saved = dup(2);
		char junk[64];
		long got = -1;
		int r1, e1, r2, e2;
		pipe(ep);
		fcntl(ep[0], F_SETFL, O_NONBLOCK);
		dup2(ep[1], 2);
		errno = 0; r1 = call_fdinfo(pid, kq, 77, &ki, sizeof ki); e1 = errno;
		errno = 0; r2 = proc_pidinfo(pid, 77, 0, junk, sizeof junk); e2 = errno;
		dup2(saved, 2);
		got = read(ep[0], junk, sizeof junk);
		CHECK("unsupported pidfdinfo flavor: EINVAL", r1 == 0 && e1 == EINVAL);
		CHECK("unsupported pidinfo flavor: EINVAL", r2 == 0 && e2 == EINVAL);
		CHECK("unsupported flavors print nothing to stderr", got < 0 && errno == EAGAIN);
		close(ep[0]); close(ep[1]); close(saved);
	}

	printf("%d failure(s)\n", failures);
	return failures;
}
