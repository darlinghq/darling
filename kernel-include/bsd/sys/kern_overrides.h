/*
 * Copyright (c) 2013 Apple Computer, Inc. All rights reserved.
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

#ifndef _SYS_KERN_OVERRIDES_H
#define _SYS_KERN_OVERRIDES_H

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/types.h>
#include <stdint.h>

__BEGIN_DECLS

/*
 * system_override() system call
 *
 * The system_override() syscall is used to modify some kernel performance mechanisms.
 * The system call needs a special entitlement and should be used with extreme caution.
 * A misuse of this syscall could lead to severe performance and battery life issues.
 *
 * The caller needs to specify the mask for the specific mechanisms to modify and a
 * timeout. The implementation of this system call blocks the thread in the syscall
 * for the duration specified in the call. Blocking a thread in the system call allows
 * the kernel to revert the modification in case the calling process dies. It also
 * makes the change of behavior extremely obvious due to the backtrace of the calling
 * thread.
 *
 * Multiple agents are allowed to call this interface at the same time. The behavior
 * change is effective from the time the first call is made (for a specific mechanism)
 * until the longest timeout specified by any agent. If the caller wishes to disable
 * the behavior change caused by itself, it can call the same interface with the
 * SYS_OVERRIDE_DISABLE flag and the mechanism mask from another thread in the same
 * process. Note that this does not break out the original thread from the block
 * immediately. It simply undoes the mechanism change underneath.
 *
 * The currently supported overrides are:
 * - SYS_OVERRIDE_IO_THROTTLE:   Modifies I/O throttling behavior
 * - SYS_OVERRIDE_CPU_THROTTLE:  Modifies background stepper throttling mechanism
 * - SYS_OVERRIDE_FAST_JETSAM:   Modifies jetsam behavior to use aggressive parallel jetsam
 *
 */

/* System Overrides Flags */
#define SYS_OVERRIDE_DISABLE            (~(~0ull >> 1))
#define SYS_OVERRIDE_IO_THROTTLE        0x1
#define SYS_OVERRIDE_CPU_THROTTLE       0x2
#define SYS_OVERRIDE_FAST_JETSAM        0x4

#define SYS_OVERRIDE_FLAGS_MASK         (SYS_OVERRIDE_DISABLE | SYS_OVERRIDE_IO_THROTTLE | SYS_OVERRIDE_CPU_THROTTLE | SYS_OVERRIDE_FAST_JETSAM)

#ifdef BSD_KERNEL_PRIVATE
void init_system_override(void);
#endif

#ifndef KERNEL
int system_override(uint64_t timeout, uint64_t flags);
#endif

__END_DECLS

#endif /*_SYS_KERN_OVERRIDES_H */
