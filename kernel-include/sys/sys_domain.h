/*
 * Copyright (c) 2000-2005 Apple Computer, Inc. All rights reserved.
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


#ifndef _SYSTEM_DOMAIN_H_
#define _SYSTEM_DOMAIN_H_


#include <sys/appleapiopts.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#ifdef KERNEL_PRIVATE
#include <sys/queue.h>
#include <libkern/locks.h>
#include <sys/kern_control.h>
#endif /* KERNEL_PRIVATE */

/* Kernel Events Protocol */ 
#define SYSPROTO_EVENT 		1	/* kernel events protocol */

/* Kernel Control Protocol */
#define SYSPROTO_CONTROL       	2	/* kernel control protocol */
#define AF_SYS_CONTROL		2	/* corresponding sub address type */

/* System family socket address */
struct sockaddr_sys
{
	u_char	ss_len;				/* sizeof(struct sockaddr_sys) */
	u_char	ss_family;			/* AF_SYSTEM */
	u_int16_t 	ss_sysaddr; 	/* protocol address in AF_SYSTEM */
	u_int32_t 	ss_reserved[7]; /* reserved to the protocol use */
};


#ifdef KERNEL_PRIVATE

/*
 * internal structure maintained for each register controller
 */
struct ctl_cb;
struct socket;

struct kctl
{
	TAILQ_ENTRY(kctl)		next;					/* controller chain */

	/* controller information provided when registering */
	char					name[MAX_KCTL_NAME];	/* unique nke identifier, provided by DTS */
	u_int32_t				id;
	u_int32_t				reg_unit;

	/* misc communication information */
	u_int32_t				flags;					/* support flags */
	u_int32_t				recvbufsize;			/* request more than the default buffer size */
	u_int32_t				sendbufsize;			/* request more than the default buffer size */

	/* Dispatch functions */
	ctl_connect_func		connect;				/* Make contact */
	ctl_disconnect_func		disconnect;				/* Break contact */
	ctl_send_func			send;					/* Send data to nke */
	ctl_setopt_func			setopt;					/* set kctl configuration */
	ctl_getopt_func			getopt;					/* get kctl configuration */

	TAILQ_HEAD(, ctl_cb)	kcb_head;
	u_int32_t				lastunit;
};

struct ctl_cb {
	TAILQ_ENTRY(ctl_cb)		next;					/* controller chain */
	lck_mtx_t				*mtx;
	struct socket			*so;					/* controlling socket */
	struct kctl				*kctl;					/* back pointer to controller */
	void					*userdata;
	u_int32_t				unit;
	u_int32_t				usecount;
};


extern struct domain systemdomain;

/* built in system domain protocols init function */
__BEGIN_DECLS
int kern_event_init(void) __attribute__((section("__TEXT, initcode")));
int kern_control_init(void) __attribute__((section("__TEXT, initcode")));
__END_DECLS

#endif /* KERNEL_PRIVATE */

#endif /* _SYSTEM_DOMAIN_H_ */


