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

#ifndef _MLDR_EXEC_POLICY_H_
#define _MLDR_EXEC_POLICY_H_

#include <stdbool.h>
#include <stdint.h>

// Which of the process's read/write memory may be executed, decided once per
// process from the main executable's Mach-O header flags (MH_NO_HEAP_EXECUTION
// and MH_ALLOW_STACK_EXECUTION in <mach-o/loader.h>).
struct mldr_exec_policy {
	// every readable mapping (heap, anonymous memory, data and bss, file-backed
	// private mappings) is also executable, as with Linux's READ_IMPLIES_EXEC
	bool exec_data;
	// the main thread stack is executable too
	bool exec_stack;
};

// is_i386 is true for a 32-bit x86 main executable.
//
// A 32-bit x86 executable without MH_NO_HEAP_EXECUTION gets an executable heap.
// An executable with MH_ALLOW_STACK_EXECUTION, of any x86 architecture, gets
// executable data and an executable stack. Everything else gets none.
struct mldr_exec_policy mldr_exec_policy_for(bool is_i386, uint32_t macho_flags);

struct load_results;

// Sets (or clears) the process's executable-memory mode for the given header
// flags, and makes the stack executable if the policy asks for it. The mode
// is inherited across fork and exec, so it has to be applied for every
// executable: with macho_flags 0 and is_i386 false it clears the mode.
void mldr_apply_exec_policy(struct load_results* lr, bool is_i386, uint32_t macho_flags);

#endif // _MLDR_EXEC_POLICY_H_
