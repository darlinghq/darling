/*
 * Copyright (c) 2000-2005, 2012, 2014 Apple Inc. All rights reserved.
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
#include <sys/sysctl.h>
#endif /* KERNEL_PRIVATE */

/* Kernel Events Protocol */
#define SYSPROTO_EVENT          1       /* kernel events protocol */

/* Kernel Control Protocol */
#define SYSPROTO_CONTROL        2       /* kernel control protocol */
#define AF_SYS_CONTROL          2       /* corresponding sub address type */

/* System family socket address */
struct sockaddr_sys {
	u_char          ss_len;         /* sizeof(struct sockaddr_sys) */
	u_char          ss_family;      /* AF_SYSTEM */
	u_int16_t       ss_sysaddr;     /* protocol address in AF_SYSTEM */
	u_int32_t       ss_reserved[7]; /* reserved to the protocol use */
};

#ifdef PRIVATE
struct  xsystmgen {
	u_int32_t       xg_len; /* length of this structure */
	u_int32_t       xg_count;       /* number of PCBs at this time */
	u_int64_t       xg_gen; /* generation count at this time */
	u_int64_t       xg_sogen;       /* current socket generation count */
};
#endif /* PRIVATE */

#ifdef KERNEL_PRIVATE

extern struct domain *systemdomain;

SYSCTL_DECL(_net_systm);

/* built in system domain protocols init function */
__BEGIN_DECLS
void kern_event_init(struct domain *);
void kern_control_init(struct domain *);
__END_DECLS
#endif /* KERNEL_PRIVATE */

#endif /* _SYSTEM_DOMAIN_H_ */
