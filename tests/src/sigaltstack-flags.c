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

// sigaltstack() flag handling: SS_DISABLE is the only accepted input flag,
// the read-back reports SS_DISABLE / 0 / SS_ONSTACK as on macOS, and a
// rejected call leaves the installed stack alone but still reports it
// through the output argument (observed on macOS 27, arm64 and x86_64).

#define _DARWIN_C_SOURCE
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failed;
#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL: " __VA_ARGS__); printf("\n"); failed = 1; } } while (0)

static volatile int handler_flags = -1;

static void handler(int sig)
{
	stack_t cur;
	(void)sig;
	if (sigaltstack(NULL, &cur) == 0)
		handler_flags = cur.ss_flags;
}

int main(void)
{
	stack_t st, old;
	size_t size = 128 * 1024;
	void* mem = malloc(size);
	int rc;

	// macOS starts with no alternate stack (SS_DISABLE). Darling's runtime
	// installs its own for the signal machinery, so only the call is checked.
	rc = sigaltstack(NULL, &old);
	CHECK(rc == 0 && (old.ss_flags == SS_DISABLE || old.ss_flags == 0), "initial: rc %d flags %d", rc, old.ss_flags);

	st.ss_sp = mem;
	st.ss_size = size;
	st.ss_flags = 0;
	rc = sigaltstack(&st, &old);
	CHECK(rc == 0 && (old.ss_flags == SS_DISABLE || old.ss_flags == 0), "install: rc %d, old flags %d", rc, old.ss_flags);

	rc = sigaltstack(NULL, &old);
	CHECK(rc == 0 && old.ss_flags == 0 && old.ss_sp == mem && old.ss_size == size,
		"after install: rc %d flags %d sp %p size %zu", rc, old.ss_flags, old.ss_sp, (size_t)old.ss_size);

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handler;
	sa.sa_flags = SA_ONSTACK;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGUSR1, &sa, NULL);
	raise(SIGUSR1);
	CHECK(handler_flags == SS_ONSTACK, "in handler: flags %d, expected SS_ONSTACK (%d)", handler_flags, SS_ONSTACK);

	int bad_flags[] = { SS_ONSTACK, 2, 8, SS_ONSTACK | SS_DISABLE, 0x100 };
	for (unsigned i = 0; i < sizeof(bad_flags) / sizeof(bad_flags[0]); i++) {
		stack_t bad = { .ss_sp = mem, .ss_size = size, .ss_flags = bad_flags[i] };
		old.ss_sp = (void*)0x1230;
		old.ss_size = 42;
		old.ss_flags = 99;
		errno = 0;
		rc = sigaltstack(&bad, &old);
		CHECK(rc == -1 && errno == EINVAL, "flags 0x%x: rc %d errno %d, expected EINVAL", bad_flags[i], rc, errno);
		CHECK(old.ss_sp == mem && old.ss_size == size && old.ss_flags == 0,
			"flags 0x%x: rejected call reported sp %p size %zu flags %d, expected the current stack",
			bad_flags[i], old.ss_sp, (size_t)old.ss_size, old.ss_flags);
		rc = sigaltstack(NULL, &old);
		CHECK(rc == 0 && old.ss_flags == 0 && old.ss_sp == mem, "flags 0x%x: installed stack changed", bad_flags[i]);
	}

	// macOS checks ss_size even with SS_DISABLE, so pass a valid one.
	stack_t dis = { .ss_sp = mem, .ss_size = size, .ss_flags = SS_DISABLE };
	rc = sigaltstack(&dis, &old);
	CHECK(rc == 0 && old.ss_flags == 0 && old.ss_sp == mem, "disable: rc %d, old flags %d", rc, old.ss_flags);
	rc = sigaltstack(NULL, &old);
	CHECK(rc == 0 && old.ss_flags == SS_DISABLE, "after disable: rc %d flags %d, expected SS_DISABLE", rc, old.ss_flags);

	free(mem);
	if (!failed)
		printf("PASS\n");
	return failed;
}
