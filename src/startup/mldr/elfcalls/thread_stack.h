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

#ifndef _MLDR_THREAD_STACK_H_
#define _MLDR_THREAD_STACK_H_

#include <stdbool.h>
#include <stddef.h>
#include "dthreads.h"

// Size of the inaccessible guard below each thread stack mldr allocates: one
// host page.
size_t dthread_guard_size(void);

/**
 * Maps a guard of `guard_size` bytes (made PROT_NONE), a stack of `stack_size`
 * bytes above it and the thread's dthread structure above the stack, and
 * initializes the structure. Stores the top of the stack in `*stack_addr`.
 * Returns the dthread structure, or NULL if the mapping fails.
 */
dthread_t dthread_structure_allocate(size_t stack_size, size_t guard_size, void** stack_addr);

#endif
