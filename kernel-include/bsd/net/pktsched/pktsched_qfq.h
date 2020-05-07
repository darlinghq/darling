/*
 * Copyright (c) 2011-2016 Apple Inc. All rights reserved.
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
 * Copyright (c) 2010 Fabio Checconi, Luigi Rizzo, Paolo Valente
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _NET_PKTSCHED_PKTSCHED_QFQ_H_
#define _NET_PKTSCHED_PKTSCHED_QFQ_H_

#ifdef PRIVATE
#include <net/pktsched/pktsched.h>
#include <net/classq/classq.h>
#include <net/classq/classq_red.h>
#include <net/classq/classq_rio.h>
#include <net/classq/classq_blue.h>
#include <net/classq/classq_sfb.h>

#ifdef __cplusplus
extern "C" {
#endif

/* qfq class flags */
#define QFCF_RED                0x0001  /* use RED */
#define QFCF_ECN                0x0002  /* use ECN with RED/BLUE/SFB */
#define QFCF_RIO                0x0004  /* use RIO */
#define QFCF_CLEARDSCP          0x0010  /* clear diffserv codepoint */
#define QFCF_BLUE               0x0100  /* use BLUE */
#define QFCF_SFB                0x0200  /* use SFB */
#define QFCF_FLOWCTL            0x0400  /* enable flow control advisories */
#define QFCF_DEFAULTCLASS       0x1000  /* default class */
#define QFCF_DELAYBASED         0x2000  /* queue sizing is delay based */
#ifdef BSD_KERNEL_PRIVATE
#define QFCF_LAZY               0x10000000 /* on-demand resource allocation */
#endif /* BSD_KERNEL_PRIVATE */

#define QFCF_USERFLAGS                                                  \
	(QFCF_RED | QFCF_ECN | QFCF_RIO | QFCF_CLEARDSCP | QFCF_BLUE |  \
	QFCF_SFB | QFCF_FLOWCTL | QFCF_DEFAULTCLASS)

#ifdef BSD_KERNEL_PRIVATE
#define QFCF_BITS \
	"\020\1RED\2ECN\3RIO\5CLEARDSCP\11BLUE\12SFB\13FLOWCTL\15DEFAULT" \
	"\35LAZY"
#else
#define QFCF_BITS \
	"\020\1RED\2ECN\3RIO\5CLEARDSCP\11BLUE\12SFB\13FLOWCTL\15DEFAULT"
#endif /* !BSD_KERNEL_PRIVATE */

#define QFQ_MAX_CLASSES         32
#define QFQ_MAX_WSHIFT          16      /* log2(max_weight) */
#define QFQ_MAX_WEIGHT          (1 << QFQ_MAX_WSHIFT)

struct qfq_classstats {
	u_int32_t               class_handle;
	u_int32_t               index;
	u_int32_t               weight;
	u_int32_t               lmax;

	u_int32_t               qlength;
	u_int32_t               qlimit;
	u_int32_t               period;
	struct pktcntr          xmitcnt;  /* transmitted packet counter */
	struct pktcntr          dropcnt;  /* dropped packet counter */

	/* RED, RIO, BLUE, SFB related info */
	classq_type_t           qtype;
	union {
		/* RIO has 3 red stats */
		struct red_stats        red[RIO_NDROPPREC];
		struct blue_stats       blue;
		struct sfb_stats        sfb;
	};
	classq_state_t          qstate;
};

#ifdef BSD_KERNEL_PRIVATE
#define QFQ_DEBUG       1       /* enable extra debugging */

/*
 * Virtual time computations.
 *
 * S, F and V are all computed in fixed point arithmetic with
 * FRAC_BITS decimal bits.
 *
 * QFQ_MAX_INDEX is the maximum index allowed for a group. We need
 * one bit per index.
 *
 * QFQ_MAX_WSHIFT is the maximum power of two supported as a weight.
 * The layout of the bits is as below:
 *
 *                 [ MTU_SHIFT ][      FRAC_BITS    ]
 *                 [ MAX_INDEX    ][ MIN_SLOT_SHIFT ]
 *				 ^.__grp->index = 0
 *				 *.__grp->slot_shift
 *
 * where MIN_SLOT_SHIFT is derived by difference from the others.
 *
 * The max group index corresponds to Lmax/w_min, where
 *	Lmax=1<<MTU_SHIFT, w_min = 1 .
 * From this, and knowing how many groups (MAX_INDEX) we want,
 * we can derive the shift corresponding to each group.
 *
 * Because we often need to compute
 *	F = S + len/w_i  and V = V + len/wsum
 * instead of storing w_i store the value
 *	inv_w = (1<<FRAC_BITS)/w_i
 * so we can do F = S + len * inv_w * wsum.
 * We use W_TOT in the formulas so we can easily move between
 * static and adaptive weight sum.
 *
 * The per-scheduler-instance data contain all the data structures
 * for the scheduler: bitmaps and bucket lists.
 */

/*
 * Shifts used for class<->group mapping.  Class weights are in the
 * range [1, QFQ_MAX_WEIGHT], we need to map each class i to the
 * group with the smallest index that can support the L_i / r_i
 * configured for the class.
 *
 * grp->qfg_index is the index of the group; and grp->qfg_slot_shift
 * is the shift for the corresponding (scaled) sigma_i.
 *
 * When computing the group index, we do (len<<FP_SHIFT)/weight,
 * then compute an FLS (which is like a log2()), and if the result
 * is below the MAX_INDEX region we use 0 (which is the same as
 * using a larger len).
 */
#define QFQ_MAX_INDEX           19
#define QFQ_MAX_WSUM            (2 * QFQ_MAX_WEIGHT)

#define QFQ_FRAC_BITS           30      /* fixed point arithmetic */
#define QFQ_ONE_FP              (1UL << QFQ_FRAC_BITS)
#define QFQ_IWSUM               (QFQ_ONE_FP / QFQ_MAX_WSUM)

#define QFQ_MTU_SHIFT           11      /* log2(max_len) */
#define QFQ_MIN_SLOT_SHIFT      (QFQ_FRAC_BITS + QFQ_MTU_SHIFT - QFQ_MAX_INDEX)

/*
 * Possible group states, also indexes for the bitmaps array in
 * struct qfq_if. We rely on ER, IR, EB, IB being numbered 0..3
 */
enum qfq_state {
	ER = 0,                         /* eligible, ready */
	IR = 1,                         /* ineligible, ready */
	EB = 2,                         /* eligible, backlogged */
	IB = 3,                         /* ineligible, backlogged */
	QFQ_MAX_STATE
};

struct qfq_group;

struct qfq_class {
	u_int32_t       cl_handle;      /* class handle */
	class_queue_t   cl_q;           /* class queue structure */
	u_int32_t       cl_qflags;      /* class queue flags */
	union {
		void            *ptr;
		struct sfb      *sfb;   /* SFB state */
	} cl_qalg;
	struct qfq_if   *cl_qif;        /* back pointer to qif */
	u_int32_t       cl_flags;       /* class flags */

	u_int64_t       cl_S, cl_F;     /* flow timestamps (exact) */
	struct qfq_class *cl_next;      /* link for the slot list */
	/*
	 * Group we belong to.  In principle we would need the index,
	 * which is log_2(lmax/weight), but we never reference it
	 * directly, only the group.
	 */
	struct qfq_group *cl_grp;
	u_int32_t       cl_inv_w;       /* QFQ_ONE_FP/weight */
	u_int32_t       cl_lmax;        /* max packet size for this flow */

	/* statistics */
	u_int32_t       cl_period;      /* backlog period */
	struct pktcntr  cl_xmitcnt;     /* transmitted packet counter */
	struct pktcntr  cl_dropcnt;     /* dropped packet counter */
};

#define cl_sfb  cl_qalg.sfb

/*
 * Group descriptor, see the paper for details.
 * Basically this contains the bucket lists.
 */
struct qfq_group {
	u_int64_t       qfg_S, qfg_F;   /* group timestamps (approx) */
	u_int8_t        qfg_slot_shift; /* slot shift */
	u_int8_t        qfg_index;      /* group index */
	u_int8_t        qfg_front;      /* index of the front slot */
	pktsched_bitmap_t qfg_full_slots; /* non-empty slots */

	/* array of lists of active classes */
	struct qfq_class **qfg_slots;
};

/*
 * qfq interface state
 */
struct qfq_if {
	struct ifclassq         *qif_ifq;       /* backpointer to ifclassq */
	u_int32_t               qif_throttle;   /* throttling level */
	u_int8_t                qif_classes;    /* # of classes in table */
	u_int8_t                qif_maxclasses; /* max # of classes in table */
	u_int8_t                qif_maxslots;   /* max # of slots */
	struct qfq_class        *qif_default;   /* default class */
	struct qfq_class        **qif_class_tbl;

	u_int64_t               qif_V;          /* precise virtual time */
	u_int32_t               qif_wsum;       /* weight sum */
#if QFQ_DEBUG
	u_int32_t               qif_i_wsum;     /* QFQ_ONE_FP/w_sum */
	u_int32_t               qif_queued;     /* debugging */
	u_int32_t               qif_emptygrp;   /* debugging */
#endif /* QFQ_DEBUG */
	pktsched_bitmap_t       qif_bitmaps[QFQ_MAX_STATE]; /* group bitmaps */
	struct qfq_group        **qif_groups;   /* the groups */
};

#define QFQIF_IFP(_qif) ((_qif)->qif_ifq->ifcq_ifp)

struct if_ifclassq_stats;

extern void qfq_init(void);
extern struct qfq_if *qfq_alloc(struct ifnet *, int);
extern int qfq_destroy(struct qfq_if *);
extern void qfq_purge(struct qfq_if *);
extern void qfq_event(struct qfq_if *, cqev_t);
extern int qfq_add_queue(struct qfq_if *, u_int32_t, u_int32_t, u_int32_t,
    u_int32_t, u_int32_t, struct qfq_class **, classq_pkt_type_t);
extern int qfq_remove_queue(struct qfq_if *, u_int32_t);
extern int qfq_get_class_stats(struct qfq_if *, u_int32_t,
    struct qfq_classstats *);
extern int qfq_enqueue(struct qfq_if *, struct qfq_class *, pktsched_pkt_t *,
    struct pf_mtag *);
extern void qfq_dequeue(struct qfq_if *, pktsched_pkt_t *);
extern int qfq_setup_ifclassq(struct ifclassq *, u_int32_t, classq_pkt_type_t);
extern int qfq_teardown_ifclassq(struct ifclassq *ifq);
extern int qfq_getqstats_ifclassq(struct ifclassq *, u_int32_t,
    struct if_ifclassq_stats *);
#endif /* BSD_KERNEL_PRIVATE */
#ifdef __cplusplus
}
#endif
#endif /* PRIVATE */
#endif /* _NET_PKTSCHED_PKTSCHED_QFQ_H_ */
