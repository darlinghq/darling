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

// x86_64: after a BSD system call other than pipe, the secondary result
// register %rdx is zero. Calls the libSystem wrappers directly so that
// nothing runs between their return and the read of %rdx. Observed on
// macOS 27 (x86_64 under Rosetta): rdx is 0 after write and getpid.

#include <stdio.h>
#include <unistd.h>

static int failures;

#if defined(__x86_64__)
static long write_rdx(int fd, const void* buf, long len, long* rax)
{
	long rdx;
	// The wrapper follows the C calling convention; %rdx holds the length
	// on entry, so a trampoline that leaves it alone returns it unchanged.
	// Step over the red zone and align the stack for the call.
	__asm__ volatile (
		"movq %%rsp, %%r12\n\t"
		"subq $128, %%rsp\n\t"
		"andq $-16, %%rsp\n\t"
		"call _write\n\t"
		"movq %%r12, %%rsp\n\t"
		"movq %%rdx, %1"
		: "=a"(*rax), "=r"(rdx), "+D"(fd), "+S"(buf), "+d"(len)
		:
		: "rcx", "r8", "r9", "r10", "r11", "r12", "memory", "cc");
	return rdx;
}
#endif

int main(void)
{
#if defined(__x86_64__)
	int fds[2];
	if (pipe(fds) != 0)
	{
		puts("FAIL: pipe");
		return 1;
	}

	long rax;
	long rdx = write_rdx(fds[1], "hello", 5, &rax);
	if (rax != 5 || rdx != 0)
	{
		printf("FAIL: write returned %ld with rdx %ld (expected 5 and 0)\n", rax, rdx);
		failures++;
	}

	// pipe() still hands back both descriptors.
	char buf[5];
	if (read(fds[0], buf, sizeof(buf)) != 5)
	{
		puts("FAIL: read from the pipe");
		failures++;
	}
	close(fds[0]);
	close(fds[1]);
#endif
	if (failures)
		return 1;
	puts("PASS");
	return 0;
}
