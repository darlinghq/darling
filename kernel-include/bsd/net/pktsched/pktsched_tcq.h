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

#ifndef _NET_PKTSCHED_PKTSCHED_TCQ_H_
#define _NET_PKTSCHED_PKTSCHED_TCQ_H_

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

#define TCQ_MAXPRI      4       /* upper limit of the number of priorities */

/* tcq class flags */
#define TQCF_RED                0x0001  /* use RED */
#define TQCF_ECN                0x0002  /* use ECN with RED/BLUE/SFB */
#define TQCF_RIO                0x0004  /* use RIO */
#define TQCF_CLEARDSCP          0x0010  /* clear diffserv codepoint */
#define TQCF_BLUE               0x0100  /* use BLUE */
#define TQCF_SFB                0x0200  /* use SFB */
#define TQCF_FLOWCTL            0x0400  /* enable flow control advisories */
#define TQCF_DEFAULTCLASS       0x1000  /* default class */
#define TQCF_DELAYBASED         0x2000  /* queue sizing is delay based */
#ifdef BSD_KERNEL_PRIVATE
#define TQCF_LAZY               0x10000000 /* on-demand resource allocation */
#endif /* BSD_KERNEL_PRIVATE */

#define TQCF_USERFLAGS                                                  \
	(TQCF_RED | TQCF_ECN | TQCF_RIO | TQCF_CLEARDSCP | TQCF_BLUE |  \
	TQCF_SFB | TQCF_FLOWCTL | TQCF_DEFAULTCLASS)

#ifdef BSD_KERNEL_PRIVATE
#define TQCF_BITS \
	"\020\1RED\2ECN\3RIO\5CLEARDSCP\11BLUE\12SFB\13FLOWCTL\15DEFAULT" \
	"\35LAZY"
#else
#define TQCF_BITS \
	"\020\1RED\2ECN\3RIO\5CLEARDSCP\11BLUE\12SFB\13FLOWCTL"
#endif /* !BSD_KERNEL_PRIVATE */

struct tcq_classstats {
	u_int32_t               class_handle;
	u_int32_t               priority;

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
struct tcq_class {
	u_int32_t       cl_handle;      /* class handle */
	class_queue_t   cl_q;           /* class queue structure */
	u_int32_t       cl_qflags;      /* class queue flags */
	union {
		void            *ptr;
		struct sfb      *sfb;   /* SFB state */
	} cl_qalg;
	int32_t         cl_pri;         /* priority */
	u_int32_t       cl_flags;       /* class flags */
	struct tcq_if   *cl_tif;        /* back pointer to tif */

	/* statistics */
	u_int32_t       cl_period;      /* backlog period */
	struct pktcntr  cl_xmitcnt;     /* transmitted packet counter */
	struct pktcntr  cl_dropcnt;     /* dropped packet counter */
};

#define cl_sfb  cl_qalg.sfb

/*
 * tcq interface state
 */
struct tcq_if {
	struct ifclassq         *tif_ifq;       /* backpointer to ifclassq */
	int                     tif_maxpri;     /* max priority in use */
	u_int32_t               tif_throttle;   /* throttling level */
	struct tcq_class        *tif_default;   /* default class */
	struct tcq_class        *tif_classes[TCQ_MAXPRI]; /* classes */
};

#define TCQIF_IFP(_tif)         ((_tif)->tif_ifq->ifcq_ifp)

struct if_ifclassq_stats;

extern void tcq_init(void);
extern struct tcq_if *tcq_alloc(struct ifnet *, int);
extern int tcq_destroy(struct tcq_if *);
extern void tcq_purge(struct tcq_if *);
extern void tcq_event(struct tcq_if *, cqev_t);
extern int tcq_add_queue(struct tcq_if *, int, u_int32_t, int, u_int32_t,
    struct tcq_class **, classq_pkt_type_t);
extern int tcq_remove_queue(struct tcq_if *, u_int32_t);
extern int tcq_get_class_stats(struct tcq_if *, u_int32_t,
    struct tcq_classstats *);
extern int tcq_enqueue(struct tcq_if *, struct tcq_class *, pktsched_pkt_t *,
    struct pf_mtag *);
extern void tcq_dequeue_tc(struct tcq_if *, mbuf_svc_class_t, pktsched_pkt_t *);
extern int tcq_setup_ifclassq(struct ifclassq *, u_int32_t, classq_pkt_type_t);
extern int tcq_teardown_ifclassq(struct ifclassq *ifq);
extern int tcq_getqstats_ifclassq(struct ifclassq *, u_int32_t qid,
    struct if_ifclassq_stats *);
#endif /* BSD_KERNEL_PRIVATE */
#ifdef __cplusplus
}
#endif
#endif /* PRIVATE */
#endif /* _NET_PKTSCHED_PKTSCHED_TCQ_H_ */
