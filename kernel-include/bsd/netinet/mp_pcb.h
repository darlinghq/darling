/*
 * Copyright (c) 2012-2017 Apple Inc. All rights reserved.
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

#ifndef _NETINET_MP_PCB_H_
#define _NETINET_MP_PCB_H_

#ifdef BSD_KERNEL_PRIVATE
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/socketvar.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <kern/locks.h>

/* Keep in sync with bsd/dev/dtrace/scripts/mptcp.d */
typedef enum mppcb_state {
	MPPCB_STATE_INUSE       = 1,
	MPPCB_STATE_DEAD        = 2,
} mppcb_state_t;

/*
 * Multipath Protocol Control Block
 */
struct mppcb {
	TAILQ_ENTRY(mppcb)      mpp_entry;      /* glue to all PCBs */
	decl_lck_mtx_data(, mpp_lock);          /* per PCB lock */
	struct mppcbinfo        *mpp_pcbinfo;   /* PCB info */
	struct mptses           *mpp_pcbe;      /* ptr to MPTCP-session */
	struct socket           *mpp_socket;    /* back pointer to socket */
	uint32_t                mpp_flags;      /* PCB flags */
	mppcb_state_t           mpp_state;      /* PCB state */
	int32_t                 mpp_inside;     /* Indicates whether or not a thread is processing MPTCP */

#if NECP
	uuid_t necp_client_uuid;
	void (*necp_cb)(void *, int, uint32_t, uint32_t, bool *);
#endif
};

static inline struct mppcb *
mpsotomppcb(struct socket *mp_so)
{
	VERIFY(SOCK_DOM(mp_so) == PF_MULTIPATH);
	return (struct mppcb *)mp_so->so_pcb;
}

/* valid values for mpp_flags */
#define MPP_ATTACHED            0x001
#define MPP_INSIDE_OUTPUT       0x002           /* MPTCP-stack is inside mptcp_subflow_output */
#define MPP_INSIDE_INPUT        0x004           /* MPTCP-stack is inside mptcp_subflow_input */
#define MPP_INPUT_HANDLE        0x008           /* MPTCP-stack is handling input */
#define MPP_WUPCALL             0x010           /* MPTCP-stack is handling a read upcall */
#define MPP_SHOULD_WORKLOOP     0x020           /* MPTCP-stack should call the workloop function */
#define MPP_SHOULD_RWAKEUP      0x040           /* MPTCP-stack should call sorwakeup */
#define MPP_SHOULD_WWAKEUP      0x080           /* MPTCP-stack should call sowwakeup */
#define MPP_CREATE_SUBFLOWS     0x100           /* This connection needs to create subflows */

static inline boolean_t
mptcp_should_defer_upcall(struct mppcb *mpp)
{
	return !!(mpp->mpp_flags & (MPP_INSIDE_OUTPUT | MPP_INSIDE_INPUT | MPP_INPUT_HANDLE | MPP_WUPCALL));
}

/*
 * Multipath PCB Information
 */
struct mppcbinfo {
	TAILQ_ENTRY(mppcbinfo)  mppi_entry;     /* glue to all PCB info */
	TAILQ_HEAD(, mppcb)     mppi_pcbs;      /* list of PCBs */
	uint32_t                mppi_count;     /* # of PCBs in list */
	struct zone             *mppi_zone;     /* zone for this PCB */
	uint32_t                mppi_size;      /* size of PCB structure */
	lck_grp_t               *mppi_lock_grp; /* lock grp */
	lck_attr_t              *mppi_lock_attr; /* lock attr */
	lck_grp_attr_t          *mppi_lock_grp_attr; /* lock grp attr */
	decl_lck_mtx_data(, mppi_lock);         /* global PCB lock */
	uint32_t (*mppi_gc)(struct mppcbinfo *); /* garbage collector func */
	uint32_t (*mppi_timer)(struct mppcbinfo *); /* timer func */
};

__BEGIN_DECLS
extern void mp_pcbinit(void);
extern void mp_pcbinfo_attach(struct mppcbinfo *);
extern int mp_pcbinfo_detach(struct mppcbinfo *);
extern int mp_pcballoc(struct socket *, struct mppcbinfo *);
extern void mp_pcbdetach(struct socket *);
extern void mp_pcbdispose(struct mppcb *);
extern void mp_gc_sched(void);
extern void mptcp_timer_sched(void);
extern void mptcp_handle_deferred_upcalls(struct mppcb *mpp, uint32_t flag);
extern int mp_getsockaddr(struct socket *mp_so, struct sockaddr **nam);
extern int mp_getpeeraddr(struct socket *mp_so, struct sockaddr **nam);
#if NECP
extern int necp_client_register_multipath_cb(pid_t pid, uuid_t client_id, struct mppcb *mpp);
extern void necp_mppcb_dispose(struct mppcb *mpp);
#endif
__END_DECLS

#endif /* BSD_KERNEL_PRIVATE */
#endif /* !_NETINET_MP_PCB_H_ */
