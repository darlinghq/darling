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

#ifndef _PKTSCHED_PKTSCHED_H_
#define _PKTSCHED_PKTSCHED_H_

#ifdef PRIVATE
#ifdef __cplusplus
extern "C" {
#endif

/* packet scheduler type */
#define PKTSCHEDT_NONE          0       /* reserved */
#define PKTSCHEDT_CBQ           1       /* cbq */
#define PKTSCHEDT_HFSC          2       /* hfsc */
#define PKTSCHEDT_PRIQ          3       /* priority queue */
#define PKTSCHEDT_FAIRQ         4       /* fairq */
#define PKTSCHEDT_TCQ           5       /* traffic class queue */
#define PKTSCHEDT_QFQ           6       /* quick fair queueing */
#define PKTSCHEDT_FQ_CODEL      7       /* Flow queues with CoDel */
#define PKTSCHEDT_MAX           8       /* should be max sched type + 1 */

#ifdef BSD_KERNEL_PRIVATE
#include <mach/mach_time.h>
#include <sys/sysctl.h>
#include <libkern/libkern.h>

/* flags for pktsched_setup */
#define PKTSCHEDF_QALG_SFB      0x01    /* use SFB */
#define PKTSCHEDF_QALG_ECN      0x02    /* enable ECN */
#define PKTSCHEDF_QALG_FLOWCTL  0x04    /* enable flow control advisories */
#define PKTSCHEDF_QALG_DELAYBASED       0x08    /* Delay based queueing */
#define PKTSCHEDF_QALG_DRIVER_MANAGED   0x10    /* driver managed */

typedef struct _pktsched_pkt_ {
	classq_pkt_t            __pkt;
	uint32_t                __plen;
#define pktsched_ptype  __pkt.cp_ptype
#define pktsched_plen   __plen
#define pktsched_pkt    __pkt
#define pktsched_pkt_mbuf       __pkt.cp_mbuf
#define pktsched_pkt_kpkt       __pkt.cp_kpkt
} pktsched_pkt_t;

#define _PKTSCHED_PKT_INIT(_p)  do {                                    \
	(_p)->pktsched_pkt = CLASSQ_PKT_INITIALIZER((_p)->pktsched_pkt);\
	(_p)->pktsched_plen = 0;                                        \
} while (0)

/* macro for timeout/untimeout */
/* use old-style timeout/untimeout */
/* dummy callout structure */
struct callout {
	void            *c_arg;                 /* function argument */
	void            (*c_func)(void *);      /* function to call */
};

#define CALLOUT_INIT(c) do {                                    \
	(void) memset((c), 0, sizeof (*(c)));                   \
} while ( /*CONSTCOND*/ 0)

#define CALLOUT_RESET(c, t, f, a) do {                          \
	(c)->c_arg = (a);                                       \
	(c)->c_func = (f);                                      \
	timeout((f), (a), (t));                                 \
} while ( /*CONSTCOND*/ 0)

#define CALLOUT_STOP(c)         untimeout((c)->c_func, (c)->c_arg)
#define CALLOUT_INITIALIZER     { NULL, NULL }

typedef void (timeout_t)(void *);

/*
 * Bitmap operations
 */
typedef u_int32_t pktsched_bitmap_t;

static inline boolean_t
pktsched_bit_tst(u_int32_t ix, pktsched_bitmap_t *pData)
{
	return *pData & (1 << ix);
}

static inline void
pktsched_bit_set(u_int32_t ix, pktsched_bitmap_t *pData)
{
	*pData |= (1 << ix);
}

static inline void
pktsched_bit_clr(u_int32_t ix, pktsched_bitmap_t *pData)
{
	*pData &= ~(1 << ix);
}

static inline pktsched_bitmap_t
pktsched_ffs(pktsched_bitmap_t pData)
{
	return ffs(pData);
}

static inline pktsched_bitmap_t
pktsched_fls(pktsched_bitmap_t pData)
{
	return (sizeof(pktsched_bitmap_t) << 3) - clz(pData);
}

static inline pktsched_bitmap_t
__fls(pktsched_bitmap_t word)
{
	VERIFY(word != 0);
	return pktsched_fls(word) - 1;
}

static inline uint32_t
pktsched_get_pkt_len(pktsched_pkt_t *pkt)
{
	return pkt->pktsched_plen;
}

/*
 * We can use mach_absolute_time which returns a 64-bit value with
 * granularity less than a microsecond even on the slowest processor.
 */
#define read_machclk()          mach_absolute_time()

/*
 * machine dependent clock
 * a 64bit high resolution time counter.
 */
extern u_int32_t machclk_freq;
extern u_int64_t machclk_per_sec;
extern u_int32_t pktsched_verbose;

SYSCTL_DECL(_net_pktsched);

struct if_ifclassq_stats;

extern void pktsched_init(void);
extern int pktsched_setup(struct ifclassq *, u_int32_t, u_int32_t,
    classq_pkt_type_t);
extern int pktsched_teardown(struct ifclassq *);
extern int pktsched_getqstats(struct ifclassq *, u_int32_t,
    struct if_ifclassq_stats *);
extern u_int64_t pktsched_abs_to_nsecs(u_int64_t);
extern u_int64_t pktsched_nsecs_to_abstime(u_int64_t);
extern void pktsched_free_pkt(pktsched_pkt_t *);
extern int pktsched_clone_pkt(pktsched_pkt_t *, pktsched_pkt_t *);
extern void pktsched_corrupt_packet(pktsched_pkt_t *pkt);
extern void pktsched_get_pkt_vars(pktsched_pkt_t *, volatile uint32_t **,
    uint64_t **, uint32_t *, uint8_t *, uint8_t *, uint32_t *);
extern uint32_t *pktsched_get_pkt_sfb_vars(pktsched_pkt_t *, uint32_t **);
extern void pktsched_pkt_encap(pktsched_pkt_t *, classq_pkt_t *);
extern mbuf_svc_class_t pktsched_get_pkt_svc(pktsched_pkt_t *);
extern struct flowadv_fcentry *pktsched_alloc_fcentry(pktsched_pkt_t *,
    struct ifnet *, int);
#endif /* BSD_KERNEL_PRIVATE */

#ifdef __cplusplus
}
#endif
#endif /* PRIVATE */
#endif /* _PKTSCHED_PKTSCHED_H_ */
