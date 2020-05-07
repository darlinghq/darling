/*
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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

#ifndef _NET_PKTSCHED_FQ_CODEL_H_
#define _NET_PKTSCHED_FQ_CODEL_H_

#ifdef PRIVATE
#include <sys/types.h>
#include <sys/param.h>

#ifdef BSD_KERNEL_PRIVATE
#include <net/flowadv.h>
#endif /* BSD_KERNEL_PRIVATE */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BSD_KERNEL_PRIVATE
struct fcl_stat {
	u_int32_t fcl_flow_control;
	u_int32_t fcl_flow_feedback;
	u_int32_t fcl_dequeue_stall;
	u_int32_t fcl_flow_control_fail;
	u_int64_t fcl_drop_overflow;
	u_int64_t fcl_drop_early;
	u_int32_t fcl_drop_memfailure;
	u_int32_t fcl_flows_cnt;
	u_int32_t fcl_newflows_cnt;
	u_int32_t fcl_oldflows_cnt;
	u_int64_t fcl_pkt_cnt;
	u_int64_t fcl_dequeue;
	u_int64_t fcl_dequeue_bytes;
	u_int64_t fcl_byte_cnt;
	u_int32_t fcl_throttle_on;
	u_int32_t fcl_throttle_off;
	u_int32_t fcl_throttle_drops;
	u_int32_t fcl_dup_rexmts;
};

/*
 * Use the top most 8 bits of flow id as the tag for set associative
 * hashing
 */

#define FQ_IF_HASH_TAG_SIZE     8
#define FQ_IF_HASH_TAG_SHIFT    24
#define FQ_IF_HASH_TAG_MASK     0xFF
#define FQ_IF_HASH_TABLE_SIZE   (1 << FQ_IF_HASH_TAG_SIZE)

/* Set the quantum to be one MTU */
#define FQ_IF_DEFAULT_QUANTUM   1500

/* Max number of service classes currently supported */
#define FQ_IF_MAX_CLASSES       10

#define FQ_IF_LARGE_FLOW_BYTE_LIMIT     15000

struct flowq;
typedef u_int32_t pktsched_bitmap_t;
struct if_ifclassq_stats;

enum fq_if_state {
	FQ_IF_ER = 0,           /* eligible, ready */
	FQ_IF_IR = 1,           /* ineligible, ready */
	FQ_IF_EB = 2,           /* eligible blocked */
	FQ_IF_IB = 3,           /* ineligible, blocked */
	FQ_IF_MAX_STATE
};

/*
 * This priority index is used for QFQ state bitmaps, lower index gets
 * higher priority
 */
#define FQ_IF_BK_SYS_INDEX      9
#define FQ_IF_BK_INDEX  8
#define FQ_IF_BE_INDEX  7
#define FQ_IF_RD_INDEX  6
#define FQ_IF_OAM_INDEX 5
#define FQ_IF_AV_INDEX  4
#define FQ_IF_RV_INDEX  3
#define FQ_IF_VI_INDEX  2
#define FQ_IF_SIG_INDEX 2
#define FQ_IF_VO_INDEX  1
#define FQ_IF_CTL_INDEX 0

typedef SLIST_HEAD(, flowq) flowq_list_t;
typedef STAILQ_HEAD(, flowq) flowq_stailq_t;
typedef struct fq_if_classq {
	u_int32_t fcl_pri;      /* class priority, lower the better */
	u_int32_t fcl_service_class;    /* service class */
	u_int32_t fcl_quantum;          /* quantum in bytes */
	u_int32_t fcl_drr_max;          /* max flows per class for DRR */
	int64_t fcl_budget;             /* budget for this classq */
	flowq_stailq_t fcl_new_flows;   /* List of new flows */
	flowq_stailq_t fcl_old_flows;   /* List of old flows */
	struct fcl_stat fcl_stat;
} fq_if_classq_t;

typedef struct fq_codel_sched_data {
	struct ifclassq *fqs_ifq;       /* back pointer to ifclassq */
	u_int64_t       fqs_target_qdelay;      /* Target queue delay (ns) */
	u_int64_t       fqs_update_interval;    /* update interval (ns) */
	flowq_list_t    fqs_flows[FQ_IF_HASH_TABLE_SIZE]; /* flows table */
	pktsched_bitmap_t       fqs_bitmaps[FQ_IF_MAX_STATE];
	u_int32_t       fqs_pkt_droplimit;      /* drop limit */
	u_int8_t        fqs_throttle;   /* throttle on or off */
	u_int8_t        fqs_flags;      /* flags */
#define FQS_DRIVER_MANAGED      0x1
	fq_if_classq_t  fqs_classq[FQ_IF_MAX_CLASSES]; /* class queues */
	struct flowadv_fclist   fqs_fclist; /* flow control state */
	struct flowq    *fqs_large_flow; /* flow has highest number of bytes */
	classq_pkt_type_t       fqs_ptype;
} fq_if_t;

#endif /* BSD_KERNEL_PRIVATE */

struct fq_codel_flowstats {
	u_int32_t       fqst_min_qdelay;
#define FQ_FLOWSTATS_OLD_FLOW   0x1
#define FQ_FLOWSTATS_NEW_FLOW   0x2
#define FQ_FLOWSTATS_LARGE_FLOW 0x4
#define FQ_FLOWSTATS_DELAY_HIGH 0x8
#define FQ_FLOWSTATS_FLOWCTL_ON 0x10
	u_int32_t       fqst_flags;
	u_int32_t       fqst_bytes;
	u_int32_t       fqst_flowhash;
};

#define FQ_IF_MAX_FLOWSTATS     20

struct fq_codel_classstats {
	u_int32_t       fcls_pri;
	u_int32_t       fcls_service_class;
	u_int32_t       fcls_quantum;
	u_int32_t       fcls_drr_max;
	int64_t         fcls_budget;
	u_int64_t       fcls_target_qdelay;
	u_int64_t       fcls_update_interval;
	u_int32_t       fcls_flow_control;
	u_int32_t       fcls_flow_feedback;
	u_int32_t       fcls_dequeue_stall;
	u_int32_t       fcls_flow_control_fail;
	u_int64_t       fcls_drop_overflow;
	u_int64_t       fcls_drop_early;
	u_int32_t       fcls_drop_memfailure;
	u_int32_t       fcls_flows_cnt;
	u_int32_t       fcls_newflows_cnt;
	u_int32_t       fcls_oldflows_cnt;
	u_int64_t       fcls_pkt_cnt;
	u_int64_t       fcls_dequeue;
	u_int64_t       fcls_dequeue_bytes;
	u_int64_t       fcls_byte_cnt;
	u_int32_t       fcls_throttle_on;
	u_int32_t       fcls_throttle_off;
	u_int32_t       fcls_throttle_drops;
	u_int32_t       fcls_dup_rexmts;
	u_int32_t       fcls_flowstats_cnt;
	struct fq_codel_flowstats fcls_flowstats[FQ_IF_MAX_FLOWSTATS];
};

#ifdef BSD_KERNEL_PRIVATE

extern void fq_codel_scheduler_init(void);
extern struct flowq *fq_if_hash_pkt(fq_if_t *, u_int32_t, mbuf_svc_class_t,
    u_int64_t, boolean_t, classq_pkt_type_t);
extern boolean_t fq_if_at_drop_limit(fq_if_t *);
extern void fq_if_drop_packet(fq_if_t *);
extern void fq_if_is_flow_heavy(fq_if_t *, struct flowq *);
extern boolean_t fq_if_add_fcentry(fq_if_t *, pktsched_pkt_t *, uint32_t,
    uint8_t, fq_if_classq_t *);
extern void fq_if_flow_feedback(fq_if_t *, struct flowq *, fq_if_classq_t *);
extern int fq_if_setup_ifclassq(struct ifclassq *ifq, u_int32_t flags,
    classq_pkt_type_t ptype);
extern int fq_if_teardown_ifclassq(struct ifclassq *ifq);
extern int fq_if_getqstats_ifclassq(struct ifclassq *ifq, u_int32_t qid,
    struct if_ifclassq_stats *ifqs);
extern void fq_if_destroy_flow(fq_if_t *, fq_if_classq_t *,
    struct flowq *);


#endif /* BSD_KERNEL_PRIVATE */

#ifdef __cplusplus
}
#endif

#endif /* PRIVATE */
#endif /* _NET_PKTSCHED_PKTSCHED_FQ_CODEL_H_ */
