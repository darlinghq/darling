/*
 * Copyright (c) 2003-2010 Apple Inc. All rights reserved.
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
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

/*
 *	File:	error_codes.c
 *	Author:	Douglas Orr, Carnegie Mellon University
 *	Date:	Mar, 1988
 *
 *      Generic error code interface
 */

#include <mach/error.h>
#include "errorlib.h"
#include "err_libkern.sub"
#include "err_iokit.sub"
#include "err_ipc.sub"
#include "err_kern.sub"
#include "err_mach_ipc.sub"
#include "err_server.sub"
#include "err_us.sub"

const struct error_system _mach_errors[err_max_system+1] = {
	/* 0; err_kern */
	{
		errlib_count(err_os_sub),
		"(operating system/?) unknown subsystem error",
		err_os_sub,
	},
	/* 1; err_us */
	{
		errlib_count(err_us_sub),
		"(user space/?) unknown subsystem error",
		err_us_sub,
	},
	/* 2; err_server */
	{
		errlib_count(err_server_sub),
		"(server/?) unknown subsystem error",
		err_server_sub,
	},
	/* 3 (& 3f); err_ipc */
	{
		errlib_count(err_ipc_sub),
		"(ipc/?) unknown subsystem error",
		err_ipc_sub,
	},
	/* 4; err_mach_ipc */
	{
		errlib_count(err_mach_ipc_sub),
		"(ipc/?) unknown subsystem error",
		err_mach_ipc_sub,
	},

	/* 0x05 */ errorlib_system_null,
	/* 0x06 */ errorlib_system_null, /* 0x07 */ errorlib_system_null,
	/* 0x08 */ errorlib_system_null, /* 0x09 */ errorlib_system_null,
	/* 0x0a */ errorlib_system_null, /* 0x0b */ errorlib_system_null,
	/* 0x0c */ errorlib_system_null, /* 0x0d */ errorlib_system_null,
	/* 0x0e */ errorlib_system_null, /* 0x0f */ errorlib_system_null,

	/* 0x10 */ errorlib_system_null, /* 0x11 */ errorlib_system_null,
	/* 0x12 */ errorlib_system_null, /* 0x13 */ errorlib_system_null,
	/* 0x14 */ errorlib_system_null, /* 0x15 */ errorlib_system_null,
	/* 0x16 */ errorlib_system_null, /* 0x17 */ errorlib_system_null,
	/* 0x18 */ errorlib_system_null, /* 0x19 */ errorlib_system_null,
	/* 0x1a */ errorlib_system_null, /* 0x1b */ errorlib_system_null,
	/* 0x1c */ errorlib_system_null, /* 0x1d */ errorlib_system_null,
	/* 0x1e */ errorlib_system_null, /* 0x1f */ errorlib_system_null,

	/* 0x20 */ errorlib_system_null, /* 0x21 */ errorlib_system_null,
	/* 0x22 */ errorlib_system_null, /* 0x23 */ errorlib_system_null,
	/* 0x24 */ errorlib_system_null, /* 0x25 */ errorlib_system_null,
	/* 0x26 */ errorlib_system_null, /* 0x27 */ errorlib_system_null,
	/* 0x28 */ errorlib_system_null, /* 0x29 */ errorlib_system_null,
	/* 0x2a */ errorlib_system_null, /* 0x2b */ errorlib_system_null,
	/* 0x2c */ errorlib_system_null, /* 0x2d */ errorlib_system_null,
	/* 0x2e */ errorlib_system_null, /* 0x2f */ errorlib_system_null,

	/* 0x30 */ errorlib_system_null, /* 0x31 */ errorlib_system_null,
	/* 0x32 */ errorlib_system_null, /* 0x33 */ errorlib_system_null,
	/* 0x34 */ errorlib_system_null, /* 0x35 */ errorlib_system_null,
	/* 0x36 */ errorlib_system_null,

	/* 0x37; err_libkern */
	{
		errlib_count(err_libkern_sub),
		"(libkern/?) unknown subsystem error",
		err_libkern_sub,
	},

	/* 0x38; err_iokit */
	{
		errlib_count(err_iokit_sub),
		"(iokit/?) unknown subsystem error",
		err_iokit_sub,
		err_iokit_sub_map,
		errlib_count(err_iokit_sub_map)
	},

	                                 /* 0x39 */ errorlib_system_null,
	/* 0x3a */ errorlib_system_null, /* 0x3b */ errorlib_system_null,
	/* 0x3c */ errorlib_system_null, /* 0x3d */ errorlib_system_null,
	/* 0x3e */ errorlib_system_null, /* 0x3f */ errorlib_system_null,
};

// int error_system_count = errlib_count(_mach_errors);
