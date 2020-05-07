/*
 * Copyright (c) 2011-2018 Apple Inc. All rights reserved.
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

#ifndef _NET_CLASSQ_IF_CLASSQ_H_
#define _NET_CLASSQ_IF_CLASSQ_H_

#ifdef PRIVATE
#define IFCQ_SC_MAX             10              /* max number of queues */

#ifdef BSD_KERNEL_PRIVATE
#include <net/classq/classq.h>

/* maximum number of packets stored across all queues */
#define IFCQ_DEFAULT_PKT_DROP_LIMIT     2048

/* classq request types */
typedef enum cqrq {
	CLASSQRQ_PURGE =        1,      /* purge all packets */
	CLASSQRQ_PURGE_SC =     2,      /* purge service class (and flow) */
	CLASSQRQ_EVENT =        3,      /* interface events */
	CLASSQRQ_THROTTLE =     4,      /* throttle packets */
	CLASSQRQ_STAT_SC =      5,      /* get service class queue stats */
} cqrq_t;

/* classq purge_sc request argument */
typedef struct cqrq_purge_sc {
	mbuf_svc_class_t        sc;     /* (in) service class */
	u_int32_t               flow;   /* (in) 0 means all flows */
	u_int32_t               packets; /* (out) purged packets */
	u_int32_t               bytes;  /* (out) purged bytes */
} cqrq_purge_sc_t;

/* classq throttle request argument */
typedef struct cqrq_throttle {
	u_int32_t               set;    /* set or get */
	u_int32_t               level;  /* (in/out) throttling level */
} cqrq_throttle_t;

/* classq service class stats request argument */
typedef struct cqrq_stat_sc {
	mbuf_svc_class_t        sc;     /* (in) service class */
	u_int32_t               packets; /* (out) packets enqueued */
	u_int32_t               bytes;  /* (out) bytes enqueued */
} cqrq_stat_sc_t;

/*
 * A token-bucket regulator limits the rate that a network driver can
 * dequeue packets from the output queue.  Modern cards are able to buffer
 * a large amount of packets and dequeue too many packets at a time.  This
 * bursty dequeue behavior makes it impossible to schedule packets by
 * queueing disciplines.  A token-bucket is used to control the burst size
 * in a device independent manner.
 */
struct tb_regulator {
	u_int64_t       tbr_rate_raw;   /* (unscaled) token bucket rate */
	u_int32_t       tbr_percent;    /* token bucket rate in percentage */
	int64_t         tbr_rate;       /* (scaled) token bucket rate */
	int64_t         tbr_depth;      /* (scaled) token bucket depth */

	int64_t         tbr_token;      /* (scaled) current token */
	int64_t         tbr_filluptime; /* (scaled) time to fill up bucket */
	u_int64_t       tbr_last;       /* last time token was updated */

	/*   needed for poll-and-dequeue */
};

/* simple token bucket meter profile */
struct tb_profile {
	u_int64_t       rate;   /* rate in bit-per-sec */
	u_int32_t       percent; /* rate in percentage */
	u_int32_t       depth;  /* depth in bytes */
};

struct ifclassq;
enum cqdq_op;
enum cqrq;

typedef int (*ifclassq_enq_func)(struct ifclassq *, classq_pkt_t *,
    boolean_t *);
typedef void  (*ifclassq_deq_func)(struct ifclassq *, classq_pkt_t *);
typedef void (*ifclassq_deq_sc_func)(struct ifclassq *, mbuf_svc_class_t,
    classq_pkt_t *);
typedef int (*ifclassq_deq_multi_func)(struct ifclassq *, u_int32_t,
    u_int32_t, classq_pkt_t *, classq_pkt_t *, u_int32_t *, u_int32_t *);
typedef int (*ifclassq_deq_sc_multi_func)(struct ifclassq *,
    mbuf_svc_class_t, u_int32_t, u_int32_t, classq_pkt_t *, classq_pkt_t *,
    u_int32_t *, u_int32_t *);
typedef int (*ifclassq_req_func)(struct ifclassq *, enum cqrq, void *);

/*
 * Structure defining a queue for a network interface.
 */
struct ifclassq {
	decl_lck_mtx_data(, ifcq_lock);

	struct ifnet    *ifcq_ifp;      /* back pointer to interface */
	u_int32_t       ifcq_len;       /* packet count */
	u_int32_t       ifcq_maxlen;
	struct pktcntr  ifcq_xmitcnt;
	struct pktcntr  ifcq_dropcnt;

	u_int32_t       ifcq_type;      /* scheduler type */
	u_int32_t       ifcq_flags;     /* flags */
	u_int32_t       ifcq_sflags;    /* scheduler flags */
	u_int32_t       ifcq_target_qdelay; /* target queue delay */
	u_int32_t       ifcq_bytes;     /* bytes count */
	u_int32_t       ifcq_pkt_drop_limit;
	void            *ifcq_disc;     /* for scheduler-specific use */
	/*
	 * ifcq_disc_slots[] represents the leaf classes configured for the
	 * corresponding discpline/scheduler, ordered by their corresponding
	 * service class index.  Each slot holds the queue ID used to identify
	 * the class instance, as well as the class instance pointer itself.
	 * The latter is used during enqueue and dequeue in order to avoid the
	 * costs associated with looking up the class pointer based on the
	 * queue ID.  The queue ID is used when querying the statistics from
	 * user space.
	 *
	 * Avoiding the use of queue ID during enqueue and dequeue is made
	 * possible by virtue of knowing the particular mbuf service class
	 * associated with the packets.  The service class index of the
	 * packet is used as the index to ifcq_disc_slots[].
	 *
	 * ifcq_disc_slots[] therefore also acts as a lookup table which
	 * provides for the mapping between MBUF_SC values and the actual
	 * scheduler classes.
	 */
	struct ifclassq_disc_slot {
		u_int32_t       qid;
		void            *cl;
	} ifcq_disc_slots[IFCQ_SC_MAX]; /* for discipline use */

	ifclassq_enq_func       ifcq_enqueue;
	ifclassq_deq_func       ifcq_dequeue;
	ifclassq_deq_sc_func    ifcq_dequeue_sc;
	ifclassq_deq_multi_func ifcq_dequeue_multi;
	ifclassq_deq_sc_multi_func ifcq_dequeue_sc_multi;
	ifclassq_req_func       ifcq_request;

	/* token bucket regulator */
	struct tb_regulator     ifcq_tbr;       /* TBR */
};

/* ifcq_flags */
#define IFCQF_READY      0x01           /* ifclassq supports discipline */
#define IFCQF_ENABLED    0x02           /* ifclassq is in use */
#define IFCQF_TBR        0x04           /* Token Bucket Regulator is in use */

#define IFCQ_IS_READY(_ifcq)            ((_ifcq)->ifcq_flags & IFCQF_READY)
#define IFCQ_IS_ENABLED(_ifcq)          ((_ifcq)->ifcq_flags & IFCQF_ENABLED)
#define IFCQ_TBR_IS_ENABLED(_ifcq)      ((_ifcq)->ifcq_flags & IFCQF_TBR)

/* classq enqueue return value */
/* packet has to be dropped */
#define CLASSQEQ_DROP           (-1)
/* packet successfully enqueued */
#define CLASSQEQ_SUCCESS        0
/* packet enqueued; give flow control feedback */
#define CLASSQEQ_SUCCESS_FC     1
/* packet needs to be dropped due to flowcontrol; give flow control feedback */
#define CLASSQEQ_DROP_FC        2
/* packet needs to be dropped due to suspension; give flow control feedback */
#define CLASSQEQ_DROP_SP        3

/* interface event argument for CLASSQRQ_EVENT */
typedef enum cqev {
	CLASSQ_EV_INIT = 0,
	CLASSQ_EV_LINK_BANDWIDTH = 1,   /* link bandwidth has changed */
	CLASSQ_EV_LINK_LATENCY = 2,     /* link latency has changed */
	CLASSQ_EV_LINK_MTU =    3,      /* link MTU has changed */
	CLASSQ_EV_LINK_UP =     4,      /* link is now up */
	CLASSQ_EV_LINK_DOWN =   5,      /* link is now down */
} cqev_t;
#endif /* BSD_KERNEL_PRIVATE */

#include <net/pktsched/pktsched_tcq.h>
#include <net/pktsched/pktsched_qfq.h>
#include <net/pktsched/pktsched_fq_codel.h>

#ifdef __cplusplus
extern "C" {
#endif
struct if_ifclassq_stats {
	u_int32_t       ifqs_len;
	u_int32_t       ifqs_maxlen;
	struct pktcntr  ifqs_xmitcnt;
	struct pktcntr  ifqs_dropcnt;
	u_int32_t       ifqs_scheduler;
	union {
		struct tcq_classstats   ifqs_tcq_stats;
		struct qfq_classstats   ifqs_qfq_stats;
		struct fq_codel_classstats      ifqs_fq_codel_stats;
	};
} __attribute__((aligned(8)));

#ifdef __cplusplus
}
#endif

#ifdef BSD_KERNEL_PRIVATE
/*
 * For ifclassq lock
 */
#define IFCQ_LOCK_ASSERT_HELD(_ifcq)                                    \
	LCK_MTX_ASSERT(&(_ifcq)->ifcq_lock, LCK_MTX_ASSERT_OWNED)

#define IFCQ_LOCK_ASSERT_NOTHELD(_ifcq)                                 \
	LCK_MTX_ASSERT(&(_ifcq)->ifcq_lock, LCK_MTX_ASSERT_NOTOWNED)

#define IFCQ_LOCK(_ifcq)                                                \
	lck_mtx_lock(&(_ifcq)->ifcq_lock)

#define IFCQ_LOCK_SPIN(_ifcq)                                           \
	lck_mtx_lock_spin(&(_ifcq)->ifcq_lock)

#define IFCQ_CONVERT_LOCK(_ifcq) do {                                   \
	IFCQ_LOCK_ASSERT_HELD(_ifcq);                                   \
	lck_mtx_convert_spin(&(_ifcq)->ifcq_lock);                      \
} while (0)

#define IFCQ_UNLOCK(_ifcq)                                              \
	lck_mtx_unlock(&(_ifcq)->ifcq_lock)

/*
 * For ifclassq operations
 */
#define IFCQ_ENQUEUE(_ifq, _p, _err, _drop) do {                        \
	(_err) = (*(_ifq)->ifcq_enqueue)(_ifq, _p, _drop);              \
} while (0)

#define IFCQ_DEQUEUE(_ifq, _p) do {                                     \
	(*(_ifq)->ifcq_dequeue)(_ifq, _p);                              \
} while (0)

#define IFCQ_DEQUEUE_SC(_ifq, _sc, _p) do {                             \
	(*(_ifq)->ifcq_dequeue_sc)(_ifq, _sc, _p);                      \
} while (0)

#define IFCQ_TBR_DEQUEUE(_ifcq, _p) do {                                \
	ifclassq_tbr_dequeue(_ifcq, _p);                                \
} while (0)

#define IFCQ_TBR_DEQUEUE_SC(_ifcq, _sc, _p) do {                        \
	ifclassq_tbr_dequeue_sc(_ifcq, _sc, _p);                        \
} while (0)

#define IFCQ_PURGE(_ifq) do {                                           \
	(void) (*(_ifq)->ifcq_request)(_ifq, CLASSQRQ_PURGE, NULL);     \
} while (0)

#define IFCQ_PURGE_SC(_ifq, _sc, _flow, _packets, _bytes) do {          \
	cqrq_purge_sc_t _req = { _sc, _flow, 0, 0 };                    \
	(void) (*(_ifq)->ifcq_request)(_ifq, CLASSQRQ_PURGE_SC, &_req); \
	(_packets) = _req.packets;                                      \
	(_bytes) = _req.bytes;                                          \
} while (0)

#define IFCQ_UPDATE(_ifq, _ev) do {                                     \
	(void) (*(_ifq)->ifcq_request)(_ifq, CLASSQRQ_EVENT,            \
	    (void *)(_ev));                                             \
} while (0)

#define IFCQ_SET_THROTTLE(_ifq, _level, _err) do {                      \
	cqrq_throttle_t _req = { 1, _level };                           \
	(_err) = (*(_ifq)->ifcq_request)                                \
	    (_ifq, CLASSQRQ_THROTTLE, &_req);                           \
} while (0)

#define IFCQ_GET_THROTTLE(_ifq, _level, _err) do {                      \
	cqrq_throttle_t _req = { 0, IFNET_THROTTLE_OFF };               \
	(_err) = (*(_ifq)->ifcq_request)                                \
	    (_ifq, CLASSQRQ_THROTTLE, &_req);                           \
	(_level) = _req.level;                                          \
} while (0)

#define IFCQ_LEN_SC(_ifq, _sc, _packets, _bytes, _err) do {             \
	cqrq_stat_sc_t _req = { _sc, 0, 0 };                            \
	(_err) = (*(ifq)->ifcq_request)(_ifq, CLASSQRQ_STAT_SC, &_req); \
	if ((_packets) != NULL)                                         \
	        (*(_packets)) = _req.packets;                           \
	if ((_bytes) != NULL)                                           \
	        (*(_bytes)) = _req.bytes;                               \
} while (0)

#define IFCQ_LEN(_ifcq)         ((_ifcq)->ifcq_len)
#define IFCQ_QFULL(_ifcq)       (IFCQ_LEN(_ifcq) >= (_ifcq)->ifcq_maxlen)
#define IFCQ_IS_EMPTY(_ifcq)    (IFCQ_LEN(_ifcq) == 0)
#define IFCQ_INC_LEN(_ifcq)     (IFCQ_LEN(_ifcq)++)
#define IFCQ_DEC_LEN(_ifcq)     (IFCQ_LEN(_ifcq)--)
#define IFCQ_MAXLEN(_ifcq)      ((_ifcq)->ifcq_maxlen)
#define IFCQ_SET_MAXLEN(_ifcq, _len) ((_ifcq)->ifcq_maxlen = (_len))
#define IFCQ_TARGET_QDELAY(_ifcq)       ((_ifcq)->ifcq_target_qdelay)
#define IFCQ_BYTES(_ifcq)       ((_ifcq)->ifcq_bytes)
#define IFCQ_INC_BYTES(_ifcq, _len)     \
    ((_ifcq)->ifcq_bytes = (_ifcq)->ifcq_bytes + (_len))
#define IFCQ_DEC_BYTES(_ifcq, _len)     \
    ((_ifcq)->ifcq_bytes = (_ifcq)->ifcq_bytes - (_len))

#define IFCQ_XMIT_ADD(_ifcq, _pkt, _len) do {                           \
	PKTCNTR_ADD(&(_ifcq)->ifcq_xmitcnt, _pkt, _len);                \
} while (0)

#define IFCQ_DROP_ADD(_ifcq, _pkt, _len) do {                           \
	PKTCNTR_ADD(&(_ifcq)->ifcq_dropcnt, _pkt, _len);                \
} while (0)

#define IFCQ_PKT_DROP_LIMIT(_ifcq)      ((_ifcq)->ifcq_pkt_drop_limit)

extern int ifclassq_setup(struct ifnet *, u_int32_t, boolean_t);
extern void ifclassq_teardown(struct ifnet *);
extern int ifclassq_pktsched_setup(struct ifclassq *);
extern void ifclassq_set_maxlen(struct ifclassq *, u_int32_t);
extern u_int32_t ifclassq_get_maxlen(struct ifclassq *);
extern int ifclassq_get_len(struct ifclassq *, mbuf_svc_class_t,
    u_int32_t *, u_int32_t *);
extern errno_t ifclassq_enqueue(struct ifclassq *, classq_pkt_t *, boolean_t *);
extern errno_t ifclassq_dequeue(struct ifclassq *, u_int32_t, u_int32_t,
    classq_pkt_t *, classq_pkt_t *, u_int32_t *, u_int32_t *);
extern errno_t ifclassq_dequeue_sc(struct ifclassq *, mbuf_svc_class_t,
    u_int32_t, u_int32_t, classq_pkt_t *, classq_pkt_t *, u_int32_t *,
    u_int32_t *);
extern void *ifclassq_poll(struct ifclassq *, classq_pkt_type_t *);
extern void *ifclassq_poll_sc(struct ifclassq *, mbuf_svc_class_t,
    classq_pkt_type_t *);
extern void ifclassq_update(struct ifclassq *, cqev_t);
extern int ifclassq_attach(struct ifclassq *, u_int32_t, void *,
    ifclassq_enq_func, ifclassq_deq_func, ifclassq_deq_sc_func,
    ifclassq_deq_multi_func, ifclassq_deq_sc_multi_func, ifclassq_req_func);
extern int ifclassq_detach(struct ifclassq *);
extern int ifclassq_getqstats(struct ifclassq *, u_int32_t,
    void *, u_int32_t *);
extern const char *ifclassq_ev2str(cqev_t);
extern int ifclassq_tbr_set(struct ifclassq *, struct tb_profile *, boolean_t);
extern void ifclassq_tbr_dequeue(struct ifclassq *, classq_pkt_t *);
extern void ifclassq_tbr_dequeue_sc(struct ifclassq *, mbuf_svc_class_t,
    classq_pkt_t *);
extern void ifclassq_calc_target_qdelay(struct ifnet *ifp,
    u_int64_t *if_target_qdelay);
extern void ifclassq_calc_update_interval(u_int64_t *update_interval);
extern void ifclassq_set_packet_metadata(struct ifclassq *ifq,
    struct ifnet *ifp, classq_pkt_t *p);
extern void ifclassq_reap_caches(boolean_t);

#endif /* BSD_KERNEL_PRIVATE */
#endif /* PRIVATE */
#endif /* _NET_CLASSQ_IF_CLASSQ_H_ */
