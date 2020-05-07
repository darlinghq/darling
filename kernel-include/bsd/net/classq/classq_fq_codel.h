/*
 * Copyright (c) 2016-2018 Apple Inc. All rights reserved.
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

#ifndef _NET_CLASSQ_CLASSQ_FQ_CODEL_H
#define _NET_CLASSQ_CLASSQ_FQ_CODEL_H
#ifdef PRIVATE
#ifdef BSD_KERNEL_PRIVATE
#include <stdbool.h>
#include <sys/time.h>
#include <net/flowadv.h>
#include <net/classq/if_classq.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FQ_MIN_FC_THRESHOLD_BYTES       7500
#define FQ_IS_DELAYHIGH(_fq_)   ((_fq_)->fq_flags & FQF_DELAY_HIGH)
#define FQ_SET_DELAY_HIGH(_fq_) do { \
	(_fq_)->fq_flags |= FQF_DELAY_HIGH; \
} while (0)
#define FQ_CLEAR_DELAY_HIGH(_fq_) do { \
	(_fq_)->fq_flags &= ~FQF_DELAY_HIGH; \
} while (0)

typedef struct flowq {
	union {
		MBUFQ_HEAD(mbufq_head) __mbufq; /* mbuf packet queue */
	} __fq_pktq_u;
#define FQF_FLOWCTL_CAPABLE 0x01 /* Use flow control instead of drop */
#define FQF_DELAY_HIGH  0x02    /* Min delay is greater than target */
#define FQF_NEW_FLOW    0x04    /* Currently on new flows queue */
#define FQF_OLD_FLOW    0x08    /* Currently on old flows queue */
#define FQF_FLOWCTL_ON  0x10    /* Currently flow controlled */
	u_int8_t        fq_flags;       /* flags */
	u_int8_t        fq_sc_index; /* service_class index */
	int16_t         fq_deficit;     /* Deficit for scheduling */
	u_int32_t       fq_bytes;       /* Number of bytes in the queue */
	u_int64_t       fq_min_qdelay; /* min queue delay for Codel */
	u_int64_t       fq_updatetime; /* next update interval */
	u_int64_t       fq_getqtime;    /* last dequeue time */
	SLIST_ENTRY(flowq) fq_hashlink; /* for flow queue hash table */
	STAILQ_ENTRY(flowq) fq_actlink; /* for new/old flow queues */
	u_int32_t       fq_flowhash;    /* Flow hash */
	classq_pkt_type_t       fq_ptype; /* Packet type */
} fq_t;

#define fq_mbufq        __fq_pktq_u.__mbufq

#define fq_empty(_q)    MBUFQ_EMPTY(&(_q)->fq_mbufq)

#define fq_enqueue(_q, _p)      MBUFQ_ENQUEUE(&(_q)->fq_mbufq, _p.cp_mbuf)

#define fq_dequeue(_q, _p) do {                                         \
	MBUFQ_DEQUEUE(&(_q)->fq_mbufq, (_p)->cp_mbuf);                  \
	if (__probable((_p)->cp_mbuf != NULL)) {                        \
	        CLASSQ_PKT_INIT_MBUF((_p), (_p)->cp_mbuf);              \
	}                                                               \
} while (0)

struct fq_codel_sched_data;
struct fq_if_classq;

/* Function definitions */
extern void fq_codel_init(void);
extern void fq_codel_reap_caches(boolean_t);
extern fq_t *fq_alloc(classq_pkt_type_t);
extern void fq_destroy(fq_t *);
extern int fq_addq(struct fq_codel_sched_data *, pktsched_pkt_t *,
    struct fq_if_classq *);
extern void fq_getq_flow(struct fq_codel_sched_data *, fq_t *,
    pktsched_pkt_t *);
extern void fq_getq_flow_internal(struct fq_codel_sched_data *,
    fq_t *, pktsched_pkt_t *);
extern void fq_head_drop(struct fq_codel_sched_data *, fq_t *);

#ifdef __cplusplus
}
#endif
#endif /* BSD_KERNEL_PRIVATE */
#endif /* PRIVATE */
#endif /* _NET_CLASSQ_CLASSQ_FQ_CODEL_H */
