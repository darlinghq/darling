/*
 * Copyright (c) 1999-2013 Apple Inc. All rights reserved.
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
 * Support for socket filter kernel extensions
 */

#ifndef NET_KEXT_NET_H
#define NET_KEXT_NET_H

#include <sys/appleapiopts.h>

#include <sys/queue.h>
#include <sys/cdefs.h>

#ifdef BSD_KERNEL_PRIVATE
/*
 * Internal implementation bits
 */
#include <sys/kpi_socketfilter.h>

struct socket;
struct sockopt;
struct inpcb;

/* Private, internal implementation functions */
extern void     sflt_init(void);
extern int      sflt_permission_check(struct inpcb *inp);
extern void     sflt_initsock(struct socket *so);
extern void     sflt_termsock(struct socket *so);
extern errno_t  sflt_attach_internal(struct socket *so, sflt_handle     handle);
extern void     sflt_notify(struct socket *so, sflt_event_t event, void *param);
extern int      sflt_ioctl(struct socket *so, u_long cmd, caddr_t data);
extern int      sflt_bind(struct socket *so, const struct sockaddr *nam);
extern int      sflt_listen(struct socket *so);
extern int      sflt_accept(struct socket *head, struct socket *so,
    const struct sockaddr *local,
    const struct sockaddr *remote);
extern int      sflt_getsockname(struct socket *so, struct sockaddr **local);
extern int      sflt_getpeername(struct socket *so, struct sockaddr **remote);
extern int      sflt_connectin(struct socket *head,
    const struct sockaddr *remote);
extern int      sflt_connectout(struct socket *so, const struct sockaddr *nam);
extern int      sflt_setsockopt(struct socket *so, struct sockopt *sopt);
extern int      sflt_getsockopt(struct socket *so, struct sockopt *sopt);
extern int      sflt_data_out(struct socket *so, const struct sockaddr  *to,
    mbuf_t *data, mbuf_t *control, sflt_data_flag_t flags);
extern int      sflt_data_in(struct socket *so, const struct sockaddr *from,
    mbuf_t *data, mbuf_t *control, sflt_data_flag_t flags);

#endif /* BSD_KERNEL_PRIVATE */

#define NFF_BEFORE              0x01
#define NFF_AFTER               0x02

#define NKE_OK                  0
#define NKE_REMOVE              (-1)

/*
 * Interface structure for inserting an installed socket NKE into an
 *  existing socket.
 * 'handle' is the NKE to be inserted, 'where' is an insertion point,
 *  and flags dictate the position of the to-be-inserted NKE relative to
 *  the 'where' NKE.  If the latter is NULL, the flags indicate "first"
 *  or "last"
 */
#pragma pack(4)

struct so_nke {
	unsigned int nke_handle;
	unsigned int nke_where;
	int nke_flags; /* NFF_BEFORE, NFF_AFTER: net/kext_net.h */
	u_int32_t reserved[4];  /* for future use */
};

#pragma pack()
#endif /* NET_KEXT_NET_H */
