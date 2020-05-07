/*
 * Copyright (c) 2006-2014 Apple Inc. All rights reserved.
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

#ifndef _FSLOG_H_
#define _FSLOG_H_

#include <sys/syslog.h>
#include <sys/mount.h>
#include <machine/limits.h>

#ifdef XNU_KERNEL_PRIVATE
/* Log information about external modification of a target process */
void fslog_extmod_msgtracer(proc_t caller, proc_t target);
#endif /* XNU_KERNEL_PRIVATE */

/* Keys used by FSLog */
#define FSLOG_KEY_LEVEL         "Level"         /* Priority level */

/* Values used by FSLog */
#define FSLOG_VAL_FACILITY      "com.apple.system.fs" /* Facility generating messages */

#endif /* !_FSLOG_H_ */
