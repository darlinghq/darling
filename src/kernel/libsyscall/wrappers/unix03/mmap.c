/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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

#include <sys/cdefs.h>

#if __DARWIN_UNIX03

#include <sys/mman.h>
#include <mach/vm_param.h>
#include <errno.h>
#include <mach/mach_init.h>
#include "stack_logging_internal.h"

void *__mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);

/*
 * mmap stub, with preemptory failures due to extra parameter checking
 * mandated for conformance.
 *
 * This is for UNIX03 only.
 */
void *
mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
	/*
	 * Preemptory failures:
	 * 
	 * o	off is not a multiple of the page size
	 * o	flags does not contain either MAP_PRIVATE or MAP_SHARED
	 * o	len is zero
	 */
	extern void cerror_nocancel(int);
	if ((off & PAGE_MASK) ||
	    (((flags & MAP_PRIVATE) != MAP_PRIVATE) &&
	     ((flags & MAP_SHARED) != MAP_SHARED)) ||
	    (len == 0)) {
		cerror_nocancel(EINVAL);
		return(MAP_FAILED);
	}

	void *ptr = __mmap(addr, len, prot, flags, fildes, off);
    
	if (__syscall_logger) {
		int stackLoggingFlags = stack_logging_type_vm_allocate;
		if (flags & MAP_ANON) {
			stackLoggingFlags |= (fildes & VM_FLAGS_ALIAS_MASK);
		} else {
			stackLoggingFlags |= stack_logging_type_mapped_file_or_shared_mem;
		}
		__syscall_logger(stackLoggingFlags, (uintptr_t)mach_task_self(), (uintptr_t)len, 0, (uintptr_t)ptr, 0);
	}

	return ptr;
}

#endif /* __DARWIN_UNIX03 */
