/*
 * Copyright (c) 2000-2007 Apple Inc. All rights reserved.
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
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 *
 * Revision 1.2  1998/09/30 21:21:00  wsanchez
 * Merged in IntelMerge1 (mburg: Intel support)
 *
 * Revision 1.1.2.1  1998/09/30 18:19:50  mburg
 * Changes for Intel port
 *
 * Revision 1.1.1.1  1998/03/07 02:25:36  wsanchez
 * Import of OSF Mach kernel (~mburg)
 *
 * Revision 1.1.2.1  1996/09/17  16:56:28  bruel
 *      created from standalone mach servers
 *      [1996/09/17  16:18:08  bruel]
 *
 * Revision 1.1.7.1  1996/04/11  13:46:28  barbou
 *      Self-Contained Mach Distribution:
 *      created.
 *      [95/12/28            barbou]
 *      [96/03/28            barbou]
 *
 * $EndLog$
 */

#ifndef _MACH_MACHINE_TYPES_H_
#define _MACH_MACHINE_TYPES_H_ 1

typedef long            dev_t;          /* device number (major+minor) */

typedef signed char     bit8_t;         /* signed 8-bit quantity */
typedef unsigned char   u_bit8_t;       /* unsigned 8-bit quantity */

typedef short           bit16_t;        /* signed 16-bit quantity */
typedef unsigned short  u_bit16_t;      /* unsigned 16-bit quantity */

typedef int             bit32_t;        /* signed 32-bit quantity */
typedef unsigned int    u_bit32_t;      /* unsigned 32-bit quantity */

/* Only 32 bits of the "bit64_t" are significant on this 32-bit machine */
typedef struct { int __val[2]; } bit64_t;       /* signed 64-bit quantity */
typedef struct { unsigned int __val[2]; } u_bit64_t;/* unsigned 64-bit quantity */
#define _SIG64_BITS     __val[0]        /* bits of interest (32) */

#endif /*  _MACH_MACHINE_TYPES_H_ */
