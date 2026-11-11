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

// fcntl protection class commands, TIOCPKT on a pseudo-terminal master and
// terminate_with_payload argument checks. Expected values were observed on
// macOS 27 (arm64 and x86_64).

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <util.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int failures;

#define CHECK(cond) do { \
	if (!(cond)) { \
		printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); \
		failures++; \
	} \
} while (0)

int main(void)
{
	char path[] = "/tmp/pclassXXXXXX";
	int fd = mkstemp(path);
	CHECK(fd >= 0);

	errno = 0;
	CHECK(fcntl(fd, F_GETPROTECTIONCLASS) == 3);
	CHECK(fcntl(fd, F_SETPROTECTIONCLASS, 4) == 0);
	errno = 0;
	CHECK(fcntl(fd, F_SETPROTECTIONCLASS, 0) == -1 && errno == EINVAL);
	errno = 0;
	CHECK(fcntl(fd, F_SETPROTECTIONCLASS, 6) == -1 && errno == EPERM);
	errno = 0;
	CHECK(fcntl(fd, F_SETPROTECTIONCLASS, 99) == -1 && errno == EINVAL);
	close(fd);
	unlink(path);

	int dir = open("/tmp", O_RDONLY);
	CHECK(fcntl(dir, F_GETPROTECTIONCLASS) == 0);
	close(dir);

	int null = open("/dev/null", O_RDWR);
	errno = 0;
	CHECK(fcntl(null, F_GETPROTECTIONCLASS) == -1 && errno == ENOTSUP);
	close(null);

	int p[2];
	CHECK(pipe(p) == 0);
	errno = 0;
	CHECK(fcntl(p[0], F_GETPROTECTIONCLASS) == -1 && errno == EBADF);
	int sock = socket(AF_UNIX, SOCK_STREAM, 0);
	errno = 0;
	CHECK(fcntl(sock, F_GETPROTECTIONCLASS) == -1 && errno == EBADF);
	errno = 0;
	CHECK(fcntl(9999, F_GETPROTECTIONCLASS) == -1 && errno == EBADF);

	// Packet mode: each read from the master starts with a status byte.
	int m, s, one = 1;
	CHECK(openpty(&m, &s, NULL, NULL, NULL) == 0);
	CHECK(ioctl(m, TIOCPKT, &one) == 0);
	CHECK(write(s, "x", 1) == 1);
	char buf[8];
	ssize_t n = read(m, buf, sizeof(buf));
	CHECK(n == 2 && buf[0] == 0 && buf[1] == 'x');
	errno = 0;
	CHECK(ioctl(s, TIOCPKT, &one) == -1 && errno == ENOTTY);

	// Only a positive pid is accepted; before, pid 0 ended this process.
	errno = 0;
	CHECK(syscall(SYS_terminate_with_payload, 0, 0, 0, NULL, 0, NULL, 0) == -1 && errno == EINVAL);
	errno = 0;
	CHECK(syscall(SYS_terminate_with_payload, -1, 0, 0, NULL, 0, NULL, 0) == -1 && errno == EINVAL);
	errno = 0;
	CHECK(syscall(SYS_terminate_with_payload, 999999, 0, 0, NULL, 0, NULL, 0) == -1 && errno == ESRCH);

	if (failures)
		return 1;
	puts("PASS");
	return 0;
}
