/*
 * Copyright (c) 2004-2006 Apple Computer, Inc. All rights reserved.
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
 * NOTE: Internal ipcs.h header; all interfaces are private; if you want this
 * same information from your own program, popen(3) the ipcs(2) command and
 * parse its output, or your program may not work on future OS releases.
 */

#ifndef _SYS_IPCS_H_
#define _SYS_IPCS_H_

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>

#define	IPCS_MAGIC	0x00000001	/* Version */

/*
 * IPCS_command
 *
 * This is the IPCS command structure used for obtaining status about the
 * System V IPC mechanisms.  All other operations are based on the per
 * subsystem (shm, msg, ipc) *ctl entry point, which can be called once
 * this information is known.
 */

struct IPCS_command {
	int		ipcs_magic;	/* Magic number for struct layout */
	int		ipcs_op;	/* Operation to perform */
	int		ipcs_cursor;	/* Cursor for iteration functions */
	int		ipcs_datalen;	/* Length of ipcs_data area */
	void		*ipcs_data;	/* OP specific data */
};

#ifdef KERNEL_PRIVATE
#include <machine/types.h>

struct user_IPCS_command {
	int		ipcs_magic;	/* Magic number for struct layout */
	int		ipcs_op;	/* Operation to perform */
	int		ipcs_cursor;	/* Cursor for iteration functions */
	int		ipcs_datalen;	/* Length of ipcs_data area */
	user_addr_t	ipcs_data;	/* OP specific data */
};

struct user32_IPCS_command {
	int		ipcs_magic;	/* Magic number for struct layout */
	int		ipcs_op;	/* Operation to perform */
	int		ipcs_cursor;	/* Cursor for iteration functions */
	int		ipcs_datalen;	/* Length of ipcs_data area */
	user32_addr_t	ipcs_data;	/* OP specific data */
};

#endif /* KERNEL_PRIVATE */

/*
 * OP code values for 'ipcs_op'
 */
#define IPCS_SHM_CONF	0x00000001	/* Obtain shared memory config */
#define IPCS_SHM_ITER	0x00000002	/* Iterate shared memory info */

#define IPCS_SEM_CONF	0x00000010	/* Obtain semaphore config */
#define IPCS_SEM_ITER	0x00000020	/* Iterate semaphore info */

#define IPCS_MSG_CONF	0x00000100	/* Obtain message queue config */
#define IPCS_MSG_ITER	0x00000200	/* Iterate message queue info */

/*
 * Sysctl oid name values
 */
#define IPCS_SHM_SYSCTL	"kern.sysv.ipcs.shm"
#define IPCS_SEM_SYSCTL	"kern.sysv.ipcs.sem"
#define IPCS_MSG_SYSCTL	"kern.sysv.ipcs.msg"


#endif	/* _SYS_IPCS_H_ */
