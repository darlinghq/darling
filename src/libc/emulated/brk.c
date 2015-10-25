/*
 * Copyright (c) 1999-2010 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*
 *	File:	brk.c
 *
 *	Unix compatibility for sbrk system call.
 *
 * HISTORY
 * 09-Mar-90  Gregg Kellogg (gk) at NeXT.
 *	include <kern/mach_interface.h> instead of <kern/mach.h>
 *
 * 14-Feb-89  Avadis Tevanian (avie) at NeXT.
 *	Total rewrite using a fixed area of VM from break region.
 */

#include <mach/mach.h>		/* for vm_allocate, vm_offset_t */
#include <mach/vm_statistics.h>
#include <errno.h>

static int sbrk_needs_init = TRUE;
static vm_size_t sbrk_region_size = 4*1024*1024; /* Well, what should it be? */
static vm_address_t sbrk_curbrk;

void *sbrk(int size)
{
	kern_return_t	ret;

	if (sbrk_needs_init) {
		sbrk_needs_init = FALSE;
		/*
		 *	Allocate a big region to simulate break region.
		 */
		ret =  vm_allocate(mach_task_self(), &sbrk_curbrk, sbrk_region_size,
				  VM_MAKE_TAG(VM_MEMORY_SBRK)|TRUE);
		if (ret != KERN_SUCCESS) {
			errno = ENOMEM;
			return((void *)-1);
		}
	}
	
	if (size <= 0)
		return((void *)sbrk_curbrk);
	else if (size > sbrk_region_size) {
		errno = ENOMEM;
		return((void *)-1);
	}
	sbrk_curbrk += size;
	sbrk_region_size -= size;
	return((void *)(sbrk_curbrk - size));
}

void *brk(void *x)
{
	errno = ENOMEM;
	return((void *)-1);
}
