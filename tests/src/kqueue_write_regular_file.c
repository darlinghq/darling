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
 * kevent(2): a regular file is always writable, so EVFILT_WRITE registered
 * on one is accepted and reports an event at once and on every later wait
 * (level triggered); EV_DISABLE stops the delivery, EV_ENABLE restores it,
 * and EV_DELETE succeeds. Observed on macOS 26.7.3: the event carries no
 * EV_ERROR and data 1 (macOS 10.14.4 and 26.7.3); a positive value is required.
 *
 * Runs on macOS and in Darling; prints PASS and exits 0.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>

static int failures;

#define CHECK(cond, ...) do { \
	if (!(cond)) { \
		failures++; \
		printf("FAIL %s:%d: ", __FILE__, __LINE__); \
		printf(__VA_ARGS__); \
		printf("\n"); \
	} \
} while (0)

static int wait_one(int kq, struct kevent *out, int secs)
{
	struct timespec ts = {secs, 0};
	return kevent(kq, NULL, 0, out, 1, &ts);
}

int main(void)
{
	char path[] = "/tmp/kqueue-write-regular-XXXXXX";
	int fd = mkstemp(path);
	struct kevent ch, out;
	int kq, r;

	if (fd < 0) {
		printf("FAIL mkstemp: %s\n", strerror(errno));
		return 1;
	}
	unlink(path);
	CHECK(write(fd, "hello", 5) == 5, "write");

	kq = kqueue();
	CHECK(kq >= 0, "kqueue: %s", strerror(errno));

	EV_SET(&ch, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
	r = kevent(kq, &ch, 1, &out, 1, &(struct timespec){2, 0});
	CHECK(r == 1, "add: kevent returned %d (errno %d)", r, errno);
	if (r == 1) {
		CHECK(out.filter == EVFILT_WRITE, "filter %d", out.filter);
		CHECK((int)out.ident == fd, "ident %lu", (unsigned long)out.ident);
		CHECK(!(out.flags & EV_ERROR), "EV_ERROR set, data %ld", (long)out.data);
		CHECK(out.data > 0, "data %ld", (long)out.data);
	}

	r = wait_one(kq, &out, 2);
	CHECK(r == 1 && out.filter == EVFILT_WRITE && !(out.flags & EV_ERROR), "second wait: r=%d", r);

	EV_SET(&ch, fd, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
	r = kevent(kq, &ch, 1, NULL, 0, NULL);
	CHECK(r == 0, "disable: r=%d errno %d", r, errno);
	r = wait_one(kq, &out, 1);
	CHECK(r == 0, "event delivered while disabled: r=%d", r);

	EV_SET(&ch, fd, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
	r = kevent(kq, &ch, 1, NULL, 0, NULL);
	CHECK(r == 0, "enable: r=%d errno %d", r, errno);
	r = wait_one(kq, &out, 2);
	CHECK(r == 1 && out.filter == EVFILT_WRITE, "after enable: r=%d", r);

	EV_SET(&ch, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	r = kevent(kq, &ch, 1, NULL, 0, NULL);
	CHECK(r == 0, "delete: r=%d errno %d", r, errno);
	r = wait_one(kq, &out, 1);
	CHECK(r == 0, "event delivered after delete: r=%d", r);

	close(kq);
	close(fd);
	if (failures == 0)
		printf("PASS\n");
	return failures != 0;
}
