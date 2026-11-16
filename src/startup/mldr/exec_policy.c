/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/personality.h>
#include "exec_policy.h"
#include "loader.h"

// <mach-o/loader.h> values, repeated so that the host-side test needs no SDK
#define MLDR_MH_ALLOW_STACK_EXECUTION 0x20000u
#define MLDR_MH_NO_HEAP_EXECUTION 0x1000000u

struct mldr_exec_policy mldr_exec_policy_for(bool is_i386, uint32_t macho_flags)
{
	struct mldr_exec_policy policy = { false, false };

	if (macho_flags & MLDR_MH_ALLOW_STACK_EXECUTION)
		policy.exec_stack = policy.exec_data = true;
	else if (is_i386 && !(macho_flags & MLDR_MH_NO_HEAP_EXECUTION))
		policy.exec_data = true;

	return policy;
}

// Executable data and stack, decided by the main executable's header flags.
//
// macOS lets a 32-bit x86 process whose executable lacks MH_NO_HEAP_EXECUTION
// execute code from its heap and from anonymous read/write memory, and lets
// a process with MH_ALLOW_STACK_EXECUTION execute from the stack as well.
// Linux has the same "readable means executable" mode for x86: the personality
// flag READ_IMPLIES_EXEC makes the kernel add PROT_EXEC to every readable
// mmap/mprotect request and to the brk heap. It is a per-process setting, so
// it also covers malloc and vm_allocate (which are mmap underneath).
// It only changes mappings made after it is set: the stack was mapped earlier
// and stays non-executable unless the header asks for that.
void mldr_apply_exec_policy(struct load_results* lr, bool is_i386, uint32_t macho_flags) {
#if defined(__i386__) || defined(__x86_64__)
	struct mldr_exec_policy policy = mldr_exec_policy_for(is_i386, macho_flags);
	int persona = personality(0xffffffff);

	if (persona != -1) {
		int wanted = policy.exec_data ? (persona | READ_IMPLIES_EXEC) : (persona & ~READ_IMPLIES_EXEC);
		if (wanted != persona && personality(wanted) == -1) {
			fprintf(stderr, "mldr: cannot change the executable memory personality: %s\n", strerror(errno));
		}
	}

	if (policy.exec_stack && lr->stack_map_size != 0) {
		if (mprotect((void*)(lr->stack_top - lr->stack_map_size), lr->stack_map_size, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
			fprintf(stderr, "mldr: cannot make the stack executable: %s\n", strerror(errno));
		}
	}
#else
	(void) lr; (void) is_i386; (void) macho_flags;
#endif
}
