/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
 
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <mach/mach.h>
 
struct page
{
	struct page*	other;
	char			pad[4096-sizeof(void*)];
};


struct page p1 = { &p1 };
struct page p2 = { &p1 };
struct page p3 = { &p1 };
struct page p4 = { &p1 };
struct page p5 = { &p1 };
struct page p6 = { &p1 };
struct page p7 = { &p1 };
struct page p8 = { &p1 };
struct page p9 = { &p1 };
struct page p10 = { &p1 };

//
// This dylib has 10 pages of data and each page will be adjusted
// by dyld if this dylib is not loaded at its preferred address.
//
bool checkRebasing()
{
	int dirtyPageCount = 0;
	vm_address_t start = (((uintptr_t)&p1) + 4095) & (-4096);
	vm_address_t end = (((uintptr_t)&p10) + 4095) & (-4096);
	task_port_t task;
	task_for_pid(mach_task_self(), getpid(), &task);

	for (vm_address_t addr = start; addr < end; addr += 4096) {
		integer_t disposition = 0;
		integer_t ref_count = 0;
		vm_map_page_query(task, start, &disposition, &ref_count);
		if ( disposition & VM_PAGE_QUERY_PAGE_DIRTY ) 
			++dirtyPageCount;
	}

	//fprintf(stderr, "dirtyPageCount=%d\n", dirtyPageCount);

	// if there are too many dirty pages, then dyld is inefficient
	return ( dirtyPageCount < 2 );
}





