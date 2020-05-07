/*
 * Copyright (c) 2007-2018 Apple Inc. All rights reserved.
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

/*	$NetBSD: altq_classq.h,v 1.7 2006/10/12 19:59:08 peter Exp $	*/
/*	$KAME: altq_classq.h,v 1.6 2003/01/07 07:33:38 kjc Exp $	*/

/*
 * Copyright (c) 1991-1997 Regents of the University of California.
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the Network Research
 *	Group at Lawrence Berkeley Laboratory.
 * 4. Neither the name of the University nor of the Laboratory may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * class queue definitions extracted from rm_class.h.
 */
#ifndef _NET_CLASSQ_CLASSQ_H_
#define _NET_CLASSQ_CLASSQ_H_

#ifdef PRIVATE
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Packet types
 */
typedef enum classq_pkt_type {
	QP_INVALID = 0,
	QP_MBUF,        /* mbuf packet */
} classq_pkt_type_t;

/*
 * Packet
 */
typedef struct classq_pkt {
	union {
		struct mbuf             *cp_mbuf;       /* mbuf packet */
	};
	classq_pkt_type_t       cp_ptype;
} classq_pkt_t;

#define CLASSQ_PKT_INITIALIZER(_p)      \
	(classq_pkt_t){ .cp_mbuf = NULL, .cp_ptype = QP_INVALID }

#define CLASSQ_PKT_INIT_MBUF(_p, _m)    do {    \
	(_p)->cp_ptype = QP_MBUF;               \
	(_p)->cp_mbuf = (_m);                   \
} while (0)


/*
 * Packet Queue types
 */
typedef enum classq_type {
	Q_DROPHEAD,
	Q_DROPTAIL,
	Q_SFB
} classq_type_t;

/*
 * Packet Queue states
 */
typedef enum classq_state {
	QS_RUNNING,
	QS_SUSPENDED
} classq_state_t;

#define DEFAULT_QLIMIT  128 /* default */

#define CLASSQ_DEQUEUE_MAX_PKT_LIMIT    2048
#define CLASSQ_DEQUEUE_MAX_BYTE_LIMIT   (1024 * 1024)

/*
 * generic packet counter
 */
struct pktcntr {
	u_int64_t       packets;
	u_int64_t       bytes;
};

#ifdef BSD_KERNEL_PRIVATE
#include <sys/mcache.h>
#include <sys/mbuf.h>
#include <sys/sysctl.h>

/*
 * Packet Queue structures and macros to manipulate them.
 */
typedef struct _class_queue_ {
	union {
		MBUFQ_HEAD(mq_head) __mbufq; /* mbuf packet queue */
	} __pktq_u;
	u_int32_t       qlen;   /* Queue length (in number of packets) */
	u_int32_t       qlim;   /* Queue limit (in number of packets*) */
	u_int64_t       qsize;  /* Approx. queue size (in number of bytes) */
	classq_type_t   qtype;  /* Queue type */
	classq_state_t  qstate; /* Queue state */
	classq_pkt_type_t       qptype; /* Packet type */
} class_queue_t;

#define qmbufq(q)       (q)->__pktq_u.__mbufq   /* Get mbuf packet queue */
#define qptype(q)       (q)->qptype             /* Get queue packet type */
#define qtype(q)        (q)->qtype              /* Get queue type */
#define qstate(q)       (q)->qstate             /* Get queue state */
#define qlimit(q)       (q)->qlim               /* Max packets to be queued */
#define qlen(q)         (q)->qlen               /* Current queue length. */
#define qsize(q)        (q)->qsize              /* Approx. bytes in queue */

#define qhead(q)        MBUFQ_FIRST(&qmbufq(q))

#define qempty(q)       (qlen(q) == 0)  /* Is the queue empty?? */
#define q_is_red(q)     (qtype(q) == Q_RED)     /* Is the queue a RED queue */
#define q_is_rio(q)     (qtype(q) == Q_RIO)     /* Is the queue a RIO queue */
#define q_is_blue(q)    (qtype(q) == Q_BLUE)    /* Is the queue a BLUE queue */
#define q_is_sfb(q)     (qtype(q) == Q_SFB)     /* Is the queue a SFB queue */
#define q_is_red_or_rio(q) (qtype(q) == Q_RED || qtype(q) == Q_RIO)
#define q_is_suspended(q) (qstate(q) == QS_SUSPENDED)

#define PKTCNTR_ADD(_cntr, _pkt, _len) do {                             \
	(_cntr)->packets += (_pkt);                                     \
	(_cntr)->bytes += (_len);                                       \
} while (0)

#define PKTCNTR_CLEAR(_cntr) do {                                       \
	(_cntr)->packets = 0;                                           \
	(_cntr)->bytes = 0;                                             \
} while (0)

/* flags for mark_ecn() */
#define CLASSQF_ECN4    0x01    /* use packet marking for IPv4 packets */
#define CLASSQF_ECN6    0x02    /* use packet marking for IPv6 packets */
#define CLASSQF_ECN     (CLASSQF_ECN4 | CLASSQF_ECN6)

extern u_int32_t classq_verbose;

SYSCTL_DECL(_net_classq);

extern void _qinit(class_queue_t *, int, int, classq_pkt_type_t);
extern void _addq(class_queue_t *, classq_pkt_t *);
extern void _addq_multi(class_queue_t *, classq_pkt_t *, classq_pkt_t *,
    u_int32_t, u_int32_t);
extern void _getq(class_queue_t *, classq_pkt_t *);
extern void _getq_all(class_queue_t *, classq_pkt_t *, classq_pkt_t *,
    u_int32_t *, u_int64_t *);
extern void _getq_tail(class_queue_t *, classq_pkt_t *);
extern void _getq_random(class_queue_t *, classq_pkt_t *);
extern void _getq_flow(class_queue_t *, classq_pkt_t *, u_int32_t);
extern void _getq_scidx_lt(class_queue_t *, classq_pkt_t *, u_int32_t);
extern void _removeq(class_queue_t *, classq_pkt_t *);
extern void _flushq(class_queue_t *);
extern void _flushq_flow(class_queue_t *, u_int32_t, u_int32_t *, u_int32_t *);

extern void classq_init(void);

#if PF_ECN
extern u_int8_t read_dsfield(struct mbuf *, struct pf_mtag *);
extern void     write_dsfield(struct mbuf *, struct pf_mtag *, u_int8_t);
extern int      mark_ecn(struct mbuf *, struct pf_mtag *, int);
#endif /* PF_ECN */
#endif /* BSD_KERNEL_PRIVATE */

#ifdef __cplusplus
}
#endif
#endif /* PRIVATE */
#endif /* _NET_CLASSQ_CLASSQ_H_ */
