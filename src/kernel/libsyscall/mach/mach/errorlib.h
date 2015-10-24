/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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
 *	File:	errorlib.h
 *	Author:	Douglas Orr, Carnegie Mellon University
 *	Date:	Mar. 1988
 *
 *	Error bases for subsytems errors.
 */

#include <mach/error.h>

#define	MACH_IPC_SEND_MOD	(err_mach_ipc|err_sub(0))
#define	MACH_IPC_RCV_MOD	(err_mach_ipc|err_sub(1))
#define	MACH_IPC_MIG_MOD	(err_mach_ipc|err_sub(2))

#define	IPC_SEND_MOD		(err_ipc|err_sub(0))
#define	IPC_RCV_MOD		(err_ipc|err_sub(1))
#define	IPC_MIG_MOD		(err_ipc|err_sub(2))

#define	SERV_NETNAME_MOD	(err_server|err_sub(0))
#define	SERV_ENV_MOD		(err_server|err_sub(1))
#define	SERV_EXECD_MOD		(err_server|err_sub(2))


#define	NO_SUCH_ERROR		"unknown error code"

struct error_subsystem {
	const char		*subsys_name;
	int			max_code;
	const char * const	*codes;
};

struct error_system {
	int				max_sub;
	const char			*bad_sub;
	const struct error_subsystem	*subsystem;
};

#include <sys/cdefs.h>

__BEGIN_DECLS
extern const struct error_system 	errors[err_max_system+1];
__END_DECLS

#define	errlib_count(s)		(sizeof(s)/sizeof(s[0]))
