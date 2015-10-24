/*
 * Copyright (c) 2012 Apple Computer, Inc. All rights reserved.
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
#include <mach/mach_init.h>
#include "stack_logging_internal.h"

/*
 * munmap stub, for stack logging of VM allocations.
 *
 * This is for UNIX03 only.
 */
extern int __munmap(void *, size_t);

int
munmap(void *addr, size_t len)
{
    int result = __munmap(addr, len);
    
    if (__syscall_logger) {
        __syscall_logger(stack_logging_type_vm_deallocate, (uintptr_t)mach_task_self(), (uintptr_t)addr, len, 0, 0);
    }
    
    return result;
}

#endif /* __DARWIN_UNIX03 */
