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

// kdebug_trace with tracing disabled, and the byte counts and errors that
// proc_pidinfo() reports. Expected values were observed on macOS 27
// (arm64 and x86_64).

#include <errno.h>
#include <libproc.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#ifndef PROC_PIDUNIQIDENTIFIERINFO
#define PROC_PIDUNIQIDENTIFIERINFO 17
struct proc_uniqidentifierinfo {
	uint8_t p_uuid[16];
	uint64_t p_uniqueid;
	uint64_t p_puniqueid;
	int32_t p_idversion;
	uint32_t p_reserve2;
	uint64_t p_reserve3;
	uint64_t p_reserve4;
};
#endif

static int failures;

#define CHECK(cond) do { \
	if (!(cond)) { \
		printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); \
		failures++; \
	} \
} while (0)

int main(void)
{
	errno = 0;
	CHECK(syscall(SYS_kdebug_trace, 0x12345678, 1, 2, 3, 4) == 0);
	errno = 0;
	CHECK(syscall(SYS_kdebug_trace64, 0x12345678, 1, 2, 3, 4) == 0);
	errno = 0;
	CHECK(syscall(SYS_kdebug_trace, 0xffffffff, 0, 0, 0, 0) == 0);

	struct proc_uniqidentifierinfo ui;
	struct proc_bsdshortinfo sbi;
	struct proc_bsdinfo bi;
	struct proc_threadinfo ti;
	uint64_t threads[16];

	CHECK(proc_pidinfo(getpid(), PROC_PIDUNIQIDENTIFIERINFO, 0, &ui, sizeof(ui)) == (int) sizeof(ui));
	CHECK(proc_pidinfo(getpid(), PROC_PIDT_SHORTBSDINFO, 0, &sbi, sizeof(sbi)) == (int) sizeof(sbi));
	CHECK(proc_pidinfo(getpid(), PROC_PIDTBSDINFO, 0, &bi, sizeof(bi)) == (int) sizeof(bi));

	int n = proc_pidinfo(getpid(), PROC_PIDLISTTHREADS, 0, threads, sizeof(threads));
	CHECK(n >= (int) sizeof(uint64_t));
	if (n >= (int) sizeof(uint64_t))
		CHECK(proc_pidinfo(getpid(), PROC_PIDTHREADINFO, threads[0], &ti, sizeof(ti)) == (int) sizeof(ti));

	// The parent's unique ID is filled in (our parent is a Darling process).
	CHECK(proc_pidinfo(getpid(), PROC_PIDUNIQIDENTIFIERINFO, 0, &ui, sizeof(ui)) == (int) sizeof(ui));
	CHECK(ui.p_uniqueid != 0);
	CHECK(ui.p_puniqueid != 0);

	// A buffer one byte short fails with ENOMEM.
	errno = 0;
	CHECK(proc_pidinfo(getpid(), PROC_PIDUNIQIDENTIFIERINFO, 0, &ui, sizeof(ui) - 1) == 0 && errno == ENOMEM);
	errno = 0;
	CHECK(proc_pidinfo(getpid(), PROC_PIDTBSDINFO, 0, &bi, sizeof(bi) - 1) == 0 && errno == ENOMEM);

	if (failures)
		return 1;
	puts("PASS");
	return 0;
}
