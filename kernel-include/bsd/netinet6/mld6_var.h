/*
 * Copyright (c) 2000-2017 Apple Inc. All rights reserved.
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

/*	$FreeBSD: src/sys/netinet6/mld6_var.h,v 1.1.2.1 2000/07/15 07:14:36 kris Exp $	*/
/*	$KAME: mld6_var.h,v 1.4 2000/03/25 07:23:54 sumikawa Exp $	*/

/*
 * Copyright (C) 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _NETINET6_MLD6_VAR_H_
#define _NETINET6_MLD6_VAR_H_
#include <sys/appleapiopts.h>

/*
 * Multicast Listener Discovery (MLD)
 * implementation-specific definitions.
 */

#ifdef PRIVATE
/*
 * Per-link MLD state.
 */
#ifndef XNU_KERNEL_PRIVATE
struct mld_ifinfo {
#else
struct mld_ifinfo_u {
#endif /* XNU_KERNEL_PRIVATE */
	uint32_t mli_ifindex;   /* interface this instance belongs to */
	uint32_t mli_version;   /* MLDv1 Host Compatibility Mode */
	uint32_t mli_v1_timer;  /* MLDv1 Querier Present timer (s) */
	uint32_t mli_v2_timer;  /* MLDv2 General Query (interface) timer (s)*/
	uint32_t mli_flags;     /* MLD per-interface flags */
	uint32_t mli_rv;        /* MLDv2 Robustness Variable */
	uint32_t mli_qi;        /* MLDv2 Query Interval (s) */
	uint32_t mli_qri;       /* MLDv2 Query Response Interval (s) */
	uint32_t mli_uri;       /* MLDv2 Unsolicited Report Interval (s) */
	uint32_t _pad;
};

#define MLIF_SILENT     0x00000001      /* Do not use MLD on this ifp */
#define MLIF_USEALLOW   0x00000002      /* Use ALLOW/BLOCK for joins/leaves */
#define MLIF_PROCESSED  0x00000004      /* Entry has been processed and can be skipped */

/*
 * MLD version tag.
 */
#define MLD_VERSION_NONE                0 /* Invalid */
#define MLD_VERSION_1                   1
#define MLD_VERSION_2                   2 /* Default */
#endif /* PRIVATE */

#ifdef BSD_KERNEL_PRIVATE
#include <sys/syslog.h>

#define MLD_DEBUG 1
#ifdef MLD_DEBUG
extern int mld_debug;
#define MLD_PRINTF(x)   do { if (mld_debug) printf x; } while (0)
#else
#define MLD_PRINTF(x)
#endif

#define MLD_RANDOM_DELAY(X)             (RandomULong() % (X) + 1)
#define MLD_MAX_STATE_CHANGES           24 /* Max pending changes per group */

/*
 * MLD per-group states.
 */
#define MLD_NOT_MEMBER                  0 /* Can garbage collect group */
#define MLD_SILENT_MEMBER               1 /* Do not perform MLD for group */
#define MLD_REPORTING_MEMBER            2 /* MLDv1 we are reporter */
#define MLD_IDLE_MEMBER                 3 /* MLDv1 we reported last */
#define MLD_LAZY_MEMBER                 4 /* MLDv1 other member reporting */
#define MLD_SLEEPING_MEMBER             5 /* MLDv1 start query response */
#define MLD_AWAKENING_MEMBER            6 /* MLDv1 group timer will start */
#define MLD_G_QUERY_PENDING_MEMBER      7 /* MLDv2 group query pending */
#define MLD_SG_QUERY_PENDING_MEMBER     8 /* MLDv2 source query pending */
#define MLD_LEAVING_MEMBER              9 /* MLDv2 dying gasp (pending last */
                                          /* retransmission of INCLUDE {}) */
/*
 * MLDv2 protocol control variables.
 */
#define MLD_RV_INIT             2       /* Robustness Variable */
#define MLD_RV_MIN              1
#define MLD_RV_MAX              7

#define MLD_QI_INIT             125     /* Query Interval (s) */
#define MLD_QI_MIN              1
#define MLD_QI_MAX              255

#define MLD_QRI_INIT            10      /* Query Response Interval (s) */
#define MLD_QRI_MIN             1
#define MLD_QRI_MAX             255

#define MLD_URI_INIT            3       /* Unsolicited Report Interval (s) */
#define MLD_URI_MIN             0
#define MLD_URI_MAX             10

#define MLD_MAX_GS_SOURCES              256 /* # of sources in rx GS query */
#define MLD_MAX_G_GS_PACKETS            8 /* # of packets to answer G/GS */
#define MLD_MAX_STATE_CHANGE_PACKETS    8 /* # of packets per state change */
#define MLD_MAX_RESPONSE_PACKETS        16 /* # of packets for general query */
#define MLD_MAX_RESPONSE_BURST          4 /* # of responses to send at once */
#define MLD_RESPONSE_BURST_INTERVAL     1 /* 1 second */

/*
 * MLD-specific mbuf flags.
 */
#define M_MLDV1         M_PROTO1        /* Packet is MLDv1 */
#define M_GROUPREC      M_PROTO3        /* mbuf chain is a group record */

/*
 * Leading space for MLDv2 reports inside MTU.
 *
 * NOTE: This differs from IGMPv3 significantly. KAME IPv6 requires
 * that a fully formed mbuf chain *without* the Router Alert option
 * is passed to ip6_output(), however we must account for it in the
 * MTU if we need to split an MLDv2 report into several packets.
 *
 * We now put the MLDv2 report header in the initial mbuf containing
 * the IPv6 header.
 */
#define MLD_MTUSPACE    (sizeof(struct ip6_hdr) + sizeof(struct mld_raopt) + \
	                 sizeof(struct icmp6_hdr))

struct mld_ifinfo {
	decl_lck_mtx_data(, mli_lock);
	uint32_t mli_refcnt;    /* reference count */
	uint32_t mli_debug;     /* see ifa_debug flags */
	LIST_ENTRY(mld_ifinfo) mli_link;
	struct ifnet *mli_ifp;  /* interface this instance belongs to */
	uint32_t mli_version;   /* MLDv1 Host Compatibility Mode */
	uint32_t mli_v1_timer;  /* MLDv1 Querier Present timer (s) */
	uint32_t mli_v2_timer;  /* MLDv2 General Query (interface) timer (s)*/
	uint32_t mli_flags;     /* MLD per-interface flags */
	uint32_t mli_rv;        /* MLDv2 Robustness Variable */
	uint32_t mli_qi;        /* MLDv2 Query Interval (s) */
	uint32_t mli_qri;       /* MLDv2 Query Response Interval (s) */
	uint32_t mli_uri;       /* MLDv2 Unsolicited Report Interval (s) */
	SLIST_HEAD(, in6_multi)  mli_relinmhead; /* released groups */
	struct ifqueue   mli_gq;        /* queue of general query responses */
	struct ifqueue   mli_v1q; /* MLDv1 message queue */
};

#define MLI_LOCK_ASSERT_HELD(_mli)                                      \
	LCK_MTX_ASSERT(&(_mli)->mli_lock, LCK_MTX_ASSERT_OWNED)

#define MLI_LOCK_ASSERT_NOTHELD(_mli)                                   \
	LCK_MTX_ASSERT(&(_mli)->mli_lock, LCK_MTX_ASSERT_NOTOWNED)

#define MLI_LOCK(_mli)                                                  \
	lck_mtx_lock(&(_mli)->mli_lock)

#define MLI_LOCK_SPIN(_mli)                                             \
	lck_mtx_lock_spin(&(_mli)->mli_lock)

#define MLI_CONVERT_LOCK(_mli) do {                                     \
	MLI_LOCK_ASSERT_HELD(_mli);                                     \
	lck_mtx_convert_spin(&(_mli)->mli_lock);                        \
} while (0)

#define MLI_UNLOCK(_mli)                                                \
	lck_mtx_unlock(&(_mli)->mli_lock)

#define MLI_ADDREF(_mli)                                                \
	mli_addref(_mli, 0)

#define MLI_ADDREF_LOCKED(_mli)                                         \
	mli_addref(_mli, 1)

#define MLI_REMREF(_mli)                                                \
	mli_remref(_mli)

/*
 * Per-link MLD context.
 */
#define MLD_IFINFO(ifp) ((ifp)->if_mli)

/*
 * MLD timer schedule parameters
 */
struct mld_tparams {
	int     qpt;    /* querier_present_timers_running6 */
	int     it;     /* interface_timers_running6 */
	int     cst;    /* current_state_timers_running6 */
	int     sct;    /* state_change_timers_running6 */
};

extern int mld_change_state(struct in6_multi *, struct mld_tparams *,
    const int);
extern struct mld_ifinfo *mld_domifattach(struct ifnet *, int);
extern void mld_domifreattach(struct mld_ifinfo *);
extern void mld_domifdetach(struct ifnet *);
extern void mld_fasttimo(void);
extern void mld_ifdetach(struct ifnet *);
extern int mld_input(struct mbuf *, int, int);
extern void mld_init(void);
extern void mld_set_timeout(struct mld_tparams *);
extern void mli_addref(struct mld_ifinfo *, int);
extern void mli_remref(struct mld_ifinfo *);
__private_extern__ void mld6_initsilent(struct ifnet *, struct mld_ifinfo *);

#ifdef SYSCTL_DECL
SYSCTL_DECL(_net_inet6_mld);
#endif

#endif /* BSD_KERNEL_PRIVATE */
#endif /* _NETINET6_MLD6_VAR_H_ */
