/*
This file is part of Darling.

Copyright (C) 2026 Darling Developers

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
#include <unistd.h>
#include "thread_stack.h"

size_t dthread_guard_size(void)
{
	// The guard must be a whole page: mprotect rounds its length up to one, so a
	// smaller reserve protects into the stack instead of sitting below it.
	return (size_t) sysconf(_SC_PAGESIZE);
}

static dthread_t dthread_structure_init(dthread_t dthread, size_t guard_size, void* stack_addr, size_t stack_size, void* base_addr, size_t total_size) {
	// the pthread signature is the address of the pthread XORed with the "pointer munge" token passed in by the kernel
	// since the LKM doesn't pass in a token, it's always zero, so the signature is equal to just the address
	dthread->sig = (uintptr_t)dthread;

	dthread->tsd[DTHREAD_TSD_SLOT_PTHREAD_SELF] = dthread;
	dthread->tsd[DTHREAD_TSD_SLOT_ERRNO] = &dthread->err_no;
	dthread->tsd[DTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS] = (void*)(uintptr_t)(DTHREAD_DEFAULT_PRIORITY);
	dthread->tsd[DTHREAD_TSD_SLOT_PTR_MUNGE] = 0;
	dthread->tl_has_custom_stack = 0;
	dthread->lock = (darwin_os_unfair_lock){0};

	dthread->stackaddr = stack_addr;
	dthread->stackbottom = (char*)stack_addr - stack_size;
	dthread->freeaddr = base_addr;
	dthread->freesize = total_size;
	dthread->guardsize = guard_size;

	dthread->cancel_state = DTHREAD_CANCEL_ENABLE | DTHREAD_CANCEL_DEFERRED;

	// technically, these next values are defaults; we don't have a way to get more info from the user
	//
	// it's not too important since the only cases where we initialize the dthread structure ourselves is when we're working with workqueues,
	// and those initialize their own dthread structures when they get them

	dthread->tl_joinable = 1;
	dthread->inherit = DTHREAD_INHERIT_SCHED;
	dthread->tl_policy = DARWIN_POLICY_TIMESHARE;

	return dthread;
}

dthread_t dthread_structure_allocate(size_t stack_size, size_t guard_size, void** stack_addr) {
	size_t total_size = guard_size + stack_size + sizeof(struct _dthread);

	// allocate our stack, guard page, and dthread structure
	void* base_addr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (base_addr == MAP_FAILED) {
		fprintf(stderr, "Failed to allocate a thread stack: %s\n", strerror(errno));
		return NULL;
	}

	// protect our guard page
	if (mprotect(base_addr, guard_size, PROT_NONE) != 0)
		fprintf(stderr, "Failed to protect the thread stack guard page: %s\n", strerror(errno));

	/**
	 * memory layout of newly allocated block:
	 *
	 * [base_addr]         [base_addr + total_size]
	 * --------------------------------------------
	 * | guard page |       stack       | dthread |
	 */

	// stack_addr points to the top of the stack (i.e. the highest address)
	*stack_addr = ((char*)base_addr) + stack_size + guard_size;

	// the dthread sits above the stack
	// (and by "above", i mean the lowest address of the dthread is the highest address of the stack)
	dthread_t dthread = (dthread_t)*stack_addr;
	// zero-out the entrire dthread structure
	memset(dthread, 0, sizeof(struct _dthread));

	return dthread_structure_init(dthread, guard_size, *stack_addr, stack_size, base_addr, total_size);
}
