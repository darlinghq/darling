/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

#ifndef _SYS_MEMORY_MAINTENANCE_H_
#define _SYS_MEMORY_MAINTENANCE_H_

/*
 * File:	sys/memory_maintenance.h
 * Author:	Samuel Gosselin [sgosselin@apple.com]
 *
 * Header file for Memory Maintenance support.
 */

/*
 * The kern.darkboot sysctl can be controlled from kexts or userspace. If
 * processes want to change the sysctl value, they require the
 * 'com.apple.kern.darkboot' entitlement.
 *
 * Operating the kern.darkboot sysctl is done via using the commands below:
 *
 *      - MEMORY_MAINTENANCE_DARK_BOOT_UNSET
 *              Unset the kern.darkboot sysctl (kern.sysctl=0).
 *      - MEMORY_MAINTENANCE_DARK_BOOT_SET
 *              Set the kern.darkboot sysctl (kern.sysctl=1).
 *      - MEMORY_MAINTENANCE_DARK_BOOT_SET_PERSISTENT
 *              Set the kern.darkboot sysctl (kern.sysctl=1) and save its
 *              value into the 'darkboot' NVRAM variable.
 *
 * Example:
 *      sysctl kern.darkboot=2
 */
#define MEMORY_MAINTENANCE_DARK_BOOT_UNSET              (0)
#define MEMORY_MAINTENANCE_DARK_BOOT_SET                (1)
#define MEMORY_MAINTENANCE_DARK_BOOT_SET_PERSISTENT     (2)

#define MEMORY_MAINTENANCE_DARK_BOOT_NVRAM_NAME         "darkboot"

#endif /* _SYS_MEMORY_MAINTENANCE_H_ */
