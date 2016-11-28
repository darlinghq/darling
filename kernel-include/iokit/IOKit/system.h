/*
 * Copyright (c) 1998-2004 Apple Computer, Inc. All rights reserved.
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
#ifndef __IOKIT_SYSTEM_H
#define __IOKIT_SYSTEM_H

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <mach/mach_types.h>
#include <mach/mach_interface.h>
#include <mach/memory_object_types.h>

#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include <IOKit/assert.h>  /* Must be before other includes of kern/assert.h */

#include <kern/kern_types.h>
#include <kern/thread.h>
#include <kern/debug.h>
#include <kern/task.h>
#include <kern/sched_prim.h>
#include <kern/locks.h>
#include <kern/queue.h>
#include <kern/ipc_mig.h>
#ifndef MACH_KERNEL_PRIVATE
#include <libkern/libkern.h>
#endif

#ifdef	KERNEL_PRIVATE
#include <kern/kalloc.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#endif /* KERNEL_PRIVATE */

#ifndef	_MISC_PROTOS_H_
extern void	_doprnt( const char *format, va_list *arg,
			void (*lputc)(char), int radix );
#endif

__END_DECLS

#endif /* !__IOKIT_SYSTEM_H */
