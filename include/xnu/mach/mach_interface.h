/*
 * Copyright (c) 2000-2010 Apple Computer, Inc. All rights reserved.
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

#ifndef	_MACH_INTERFACE_H_
#define _MACH_INTERFACE_H_

#include <mach/clock.h>
#include <mach/clock_priv.h>
#include <mach/clock_reply_server.h>
#include <mach/exc_server.h>
#include <mach/host_priv.h>
#include <mach/host_security.h>
#include <mach/lock_set.h>
#include <mach/mach_exc_server.h>
#include <mach/mach_host.h>
#include <mach/mach_port.h>
#include <mach/notify_server.h>
#include <mach/processor.h>
#include <mach/processor_set.h>
#include <mach/semaphore.h>
#include <mach/task.h>
#include <mach/thread_act.h>
#include <mach/vm_map.h>

#ifdef XNU_KERNEL_PRIVATE
/*
 * Raw EMMI interfaces are private to xnu
 * and subject to change.
 */
#include <mach/memory_object_default_server.h>
#include <mach/memory_object_control.h>
#include <mach/memory_object_name.h>
#include <mach/upl.h>
#endif

#endif /* _MACH_INTERFACE_H_ */
