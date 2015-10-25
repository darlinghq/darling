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
 *	File:	sys/aio_kern.h
 *	Author:	Jerry Cottingham [jerryc@apple.com]
 *
 *	Header file for kernel only portion of POSIX Asynchronous IO APIs
 *
 */ 

#include <sys/aio.h>

#ifndef _SYS_AIO_KERN_H_
#define	_SYS_AIO_KERN_H_

#ifdef KERNEL

struct aio_workq_entry
{
	TAILQ_ENTRY( aio_workq_entry )	aio_workq_link; /* Protected by queue lock */
	TAILQ_ENTRY( aio_workq_entry )  aio_proc_link;  /* Proteced by proc's aio lock */

	/* Proc lock */
	void		*group_tag;	/* identifier used to group IO requests */

	/* Initialized and never changed, safe to access */
	struct proc	*procp;		/* user proc that queued this request */
	user_addr_t	uaiocbp;  	/* pointer passed in from user land */
	struct user_aiocb	aiocb;	/* copy of aiocb from user land */
	vm_map_t	aio_map;	/* user land map we have a reference to */

	/* Entry lock */
	int		aio_refcount;
	user_ssize_t	returnval;	/* return value from read / write request */	
	int		errorval;	/* error value from read / write request */
	int		flags;		
};
typedef struct aio_workq_entry aio_workq_entry;

/*
 * definitions for aio_workq_entry.flags
 */
#define	AIO_READ		0x00000001	/* a read */
#define	AIO_WRITE		0x00000002	/* a write */
#define	AIO_FSYNC		0x00000004	/* aio_fsync with op = O_SYNC */
#define	AIO_DSYNC		0x00000008	/* aio_fsync with op = O_DSYNC (not supported yet) */
#define	AIO_LIO			0x00000010	/* lio_listio generated IO */
#define	AIO_DO_FREE		0x00000800	/* entry should be freed when last reference is dropped. */
						/* 	set by aio_return() and _aio_exit() */
#define	AIO_DISABLE		0x00002000	/* process is trying to exit or exec and we need */
						/* 	to not try to send a signal from do_aio_completion() */
#define	AIO_CLOSE_WAIT		0x00004000	/* process is trying to close and is */
						/* 	waiting for one or more active IO requests to */
						/*	complete */
#define AIO_EXIT_WAIT		0x00008000	/* process is trying to exit or exec and is */
						/* 	waiting for one or more active IO requests to */
						/*	complete */

#define AIO_LIO_NOTIFY		0x00010000	/* wait for list complete */

/*
 * Prototypes
 */

__private_extern__ void		
_aio_close(struct proc *p, int fd);

__private_extern__ void		
_aio_exit(struct proc *p);

__private_extern__ void		
_aio_exec(struct proc *p);

__private_extern__ void		
_aio_create_worker_threads(int num);

__private_extern__ void		
aio_init(void) __attribute__((section("__TEXT, initcode")));

task_t						
get_aiotask(void);

#endif /* KERNEL */

#endif /* _SYS_AIO_KERN_H_ */
