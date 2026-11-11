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

// gettimeofday's mach time argument (mach_get_times), connectx and
// disconnectx on a loopback TCP socket, proc_pid_rusage(),
// PROC_PIDVNODEPATHINFO and the kern.maxfiles family. Checked on macOS 27
// (arm64 and x86_64).

#include <arpa/inet.h>
#include <errno.h>
#include <libproc.h>
#include <mach/mach_time.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <unistd.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

extern kern_return_t mach_get_times(uint64_t* absolute_time, uint64_t* continuous_time, struct timespec* tp);

static int failures;

#define CHECK(cond) do { \
	if (!(cond)) { \
		printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); \
		failures++; \
	} \
} while (0)

static int sysctl_int(const char* name)
{
	int value = -1;
	size_t len = sizeof(value);
	if (sysctlbyname(name, &value, &len, NULL, 0) != 0 || len != sizeof(value))
		return -1;
	return value;
}

int main(void)
{
	// gettimeofday with the third (mach time) argument
	struct timeval tv;
	uint64_t mt = 0, before = mach_absolute_time();
	CHECK(syscall(SYS_gettimeofday, &tv, NULL, &mt) == 0);
	CHECK(mt >= before && mt <= mach_absolute_time());
	CHECK(tv.tv_sec > 1600000000);

	uint64_t abs_time = 0;
	struct timespec ts = { 0, 0 };
	CHECK(mach_get_times(&abs_time, NULL, &ts) == KERN_SUCCESS);
	CHECK(abs_time != 0 && ts.tv_sec > 1600000000);

	// connectx/disconnectx
	int srv = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	socklen_t slen = sizeof(sin);
	CHECK(bind(srv, (struct sockaddr*) &sin, sizeof(sin)) == 0);
	CHECK(listen(srv, 1) == 0);
	CHECK(getsockname(srv, (struct sockaddr*) &sin, &slen) == 0);

	int cli = socket(AF_INET, SOCK_STREAM, 0);
	sa_endpoints_t ep;
	memset(&ep, 0, sizeof(ep));
	ep.sae_dstaddr = (struct sockaddr*) &sin;
	ep.sae_dstaddrlen = sizeof(sin);
	char msg[] = "hello";
	struct iovec iov = { msg, 5 };
	size_t sent = 0;
	sae_connid_t cid = 0;
	errno = 0;
	CHECK(connectx(cli, &ep, SAE_ASSOCID_ANY, 0, &iov, 1, &sent, &cid) == 0);
	CHECK(sent == 5);

	int acc = accept(srv, NULL, NULL);
	CHECK(acc >= 0);
	char buf[8] = { 0 };
	CHECK(read(acc, buf, sizeof(buf)) == 5 && memcmp(buf, "hello", 5) == 0);
	CHECK(disconnectx(cli, SAE_ASSOCID_ANY, SAE_CONNID_ANY) == 0);
	CHECK(read(acc, buf, sizeof(buf)) == 0);
	close(acc);
	close(cli);
	close(srv);

	errno = 0;
	CHECK(connectx(socket(AF_INET, SOCK_STREAM, 0), NULL, SAE_ASSOCID_ANY, 0, NULL, 0, NULL, NULL) == -1 && errno == EINVAL);

	// proc_pid_rusage
	volatile uint64_t spin = 0;
	for (uint64_t i = 0; i < 200000000ULL; i++)
		spin += i;
	struct rusage_info_v4 ri;
	memset(&ri, 0xff, sizeof(ri));
	CHECK(proc_pid_rusage(getpid(), RUSAGE_INFO_V4, (rusage_info_t*) &ri) == 0);
	CHECK(ri.ri_user_time > 0 && ri.ri_user_time != UINT64_MAX);
	// The resident size is a real resident size, not the virtual size.
	CHECK(ri.ri_resident_size > 0 && ri.ri_resident_size < (1ULL << 32));

	// PROC_PIDVNODEPATHINFO: the current directory
	CHECK(chdir("/tmp") == 0);
	char cwd[PATH_MAX];
	CHECK(getcwd(cwd, sizeof(cwd)) != NULL);
	struct proc_vnodepathinfo vpi;
	CHECK(proc_pidinfo(getpid(), PROC_PIDVNODEPATHINFO, 0, &vpi, sizeof(vpi)) == (int) sizeof(vpi));
	CHECK(strcmp(vpi.pvi_cdir.vip_path, cwd) == 0);

	// kern.maxfiles family: INT values
	CHECK(sysctl_int("kern.maxfiles") > 0);
	CHECK(sysctl_int("kern.maxfilesperproc") > 0);
	CHECK(sysctl_int("kern.maxfilesperproc") <= sysctl_int("kern.maxfiles"));
	CHECK(sysctl_int("kern.num_files") > 0);

	if (failures)
		return 1;
	puts("PASS");
	return 0;
}
