
/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
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
 *	Copyright (c) 1990, 1996-1998 Apple Computer, Inc.
 *	All Rights Reserved.
 */
/*
 * posix_shm.c : Support for POSIX semaphore APIs
 *
 *	File:	posix_sem.c
 *	Author:	Ananthakrishna Ramesh
 *
 * HISTORY
 * 2-Sep-1999	A.Ramesh
 *	Created for MacOSX
 *
 */

#ifndef	_SYS_POSIX_SEM_H_
#define _SYS_POSIX_SEM_H_

#include <sys/appleapiopts.h>
#include <sys/types.h>
#include <sys/proc.h>

struct label;

#define	PSEMNAMLEN	31	/* maximum name segment length we bother with */

struct pseminfo {
	unsigned int	psem_flags;
	unsigned int	psem_usecount;
	mode_t		psem_mode;
	uid_t		psem_uid;
	gid_t		psem_gid;
	char		psem_name[PSEMNAMLEN + 1];	/* segment name */
	void *		psem_semobject;
	struct proc *	sem_proc;
	struct label *  psem_label;
};

#define PSEMINFO_NULL (struct pseminfo *)0

#define	PSEM_NONE	1
#define	PSEM_DEFINED	2
#define	PSEM_ALLOCATED	4
#define	PSEM_MAPPED	8
#define	PSEM_INUSE	0x10
#define	PSEM_REMOVED	0x20
#define	PSEM_INCREATE	0x40
#define	PSEM_INDELETE	0x80

#endif
