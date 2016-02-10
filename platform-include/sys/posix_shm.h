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
 * posix_shm.c : Support for POSIX shared memory APIs
 *
 *	File:	posix_shm.c
 *	Author:	Ananthakrishna Ramesh
 *
 * HISTORY
 * 2-Sep-1999	A.Ramesh
 *	Created for MacOSX
 *
 */

#ifndef	_SYS_POSIX_SHM_H_
#define _SYS_POSIX_SHM_H_

#include <sys/appleapiopts.h>
#include <sys/types.h>
#include <sys/proc.h>

struct label;

#define	PSHMNAMLEN	31	/* maximum name segment length we bother with */

struct pshminfo {
	unsigned int	pshm_flags;
	unsigned int	pshm_usecount;
	off_t		pshm_length;
	mode_t		pshm_mode;
	uid_t		pshm_uid;
	gid_t		pshm_gid;
	char		pshm_name[PSHMNAMLEN + 1];	/* segment name */
	void *		pshm_memobject;
#if DIAGNOSTIC
	unsigned int 	pshm_readcount;
	unsigned int 	pshm_writecount;
	struct proc *	pshm_proc;
#endif /* DIAGNOSTIC */
	struct label *	pshm_label;
};
#define PSHMINFO_NULL (struct pshminfo *)0

#define	PSHM_NONE	1
#define	PSHM_DEFINED	2
#define	PSHM_ALLOCATED	4
#define	PSHM_MAPPED	8
#define	PSHM_INUSE	0x10
#define	PSHM_REMOVED	0x20
#define	PSHM_INCREATE	0x40
#define	PSHM_INDELETE	0x80

#endif
