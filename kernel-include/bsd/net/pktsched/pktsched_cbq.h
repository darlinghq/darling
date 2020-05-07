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

/*	$NetBSD: altq_cbq.h,v 1.8 2006/10/12 19:59:08 peter Exp $	*/
/*	$KAME: altq_cbq.h,v 1.12 2003/10/03 05:05:15 kjc Exp $	*/

/*
 * Copyright (c) Sun Microsystems, Inc. 1993-1998 All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the SMCC Technology
 *      Development Group at Sun Microsystems, Inc.
 *
 * 4. The name of the Sun Microsystems, Inc nor may not be used to endorse or
 *      promote products derived from this software without specific prior
 *      written permission.
 *
 * SUN MICROSYSTEMS DOES NOT CLAIM MERCHANTABILITY OF THIS SOFTWARE OR THE
 * SUITABILITY OF THIS SOFTWARE FOR ANY PARTICULAR PURPOSE.  The software is
 * provided "as is" without express or implied warranty of any kind.
 *
 * These notices must be retained in any copies of any part of this software.
 */

#ifndef _NET_PKTSCHED_PKTSCHED_CBQ_H_
#define _NET_PKTSCHED_PKTSCHED_CBQ_H_

#ifdef PRIVATE
#include <net/pktsched/pktsched_rmclass.h>
#include <net/classq/classq.h>
#include <net/classq/classq_red.h>
#include <net/classq/classq_rio.h>
#include <net/classq/classq_blue.h>
#include <net/classq/classq_sfb.h>

#ifdef __cplusplus
extern "C" {
#endif

/* class flags should be same as class flags in rm_class.h */
#define CBQCLF_RED              RMCF_RED        /* use RED */
#define CBQCLF_ECN              RMCF_ECN        /* use ECN with RED/BLUE/SFB */
#define CBQCLF_RIO              RMCF_RIO        /* use RIO */
#define CBQCLF_FLOWVALVE        RMCF_FLOWVALVE  /* use flowvalve/penalty-box */
#define CBQCLF_CLEARDSCP        RMCF_CLEARDSCP  /* clear diffserv codepoint */
#define CBQCLF_BORROW           0x0020          /* borrow from parent */

/* class flags only for root class */
#define CBQCLF_WRR              RMCF_WRR        /* weighted-round robin */
#define CBQCLF_EFFICIENT        RMCF_EFFICIENT  /* work-conserving */

/* class flags for special classes */
#define CBQCLF_ROOTCLASS        0x1000          /* root class */
#define CBQCLF_DEFCLASS         0x2000          /* default class */
#define CBQCLF_CLASSMASK        0xf000          /* class mask */

#define CBQCLF_BLUE             RMCF_BLUE       /* use BLUE */
#define CBQCLF_SFB              RMCF_SFB        /* use SFB */
#define CBQCLF_FLOWCTL          RMCF_FLOWCTL    /* enable flow ctl advisories */

#ifdef BSD_KERNEL_PRIVATE
#define CBQCLF_LAZY             0x10000000 /* on-demand resource allocation */
#endif /* BSD_KERNEL_PRIVATE */

#define CBQCLF_USERFLAGS                                                   \
	(CBQCLF_RED | CBQCLF_ECN | CBQCLF_RIO | CBQCLF_FLOWVALVE |         \
	CBQCLF_CLEARDSCP | CBQCLF_BORROW | CBQCLF_WRR | CBQCLF_EFFICIENT | \
	CBQCLF_ROOTCLASS | CBQCLF_DEFCLASS | CBQCLF_BLUE | CBQCLF_SFB |    \
	CBQCLF_FLOWCTL)

#ifdef BSD_KERNEL_PRIVATE
#define CBQCLF_BITS \
	"\020\1RED\2ECN\3RIO\4FLOWVALVE\5CLEARDSCP\6BORROW" \
	"\11WRR\12EFFICIENT\15ROOT\16DEFAULT\21BLUE\22SFB\23FLOWCTL\35LAZY"
#else
#define CBQCLF_BITS \
	"\020\1RED\2ECN\3RIO\4FLOWVALVE\5CLEARDSCP\6BORROW" \
	"\11WRR\12EFFICIENT\15ROOT\16DEFAULT\21BLUE\22SFB\23FLOWCTL"
#endif /* !BSD_KERNEL_PRIVATE */

#define CBQ_MAXQSIZE            200
#define CBQ_MAXPRI              RM_MAXPRIO

typedef struct cbq_classstats {
	u_int32_t       handle;
	u_int32_t       depth;

	struct pktcntr  xmit_cnt;       /* packets sent in this class */
	struct pktcntr  drop_cnt;       /* dropped packets */
	u_int32_t       over;           /* # times went over limit */
	u_int32_t       borrows;        /* # times tried to borrow */
	u_int32_t       overactions;    /* # times invoked overlimit action */
	u_int32_t       delays;         /* # times invoked delay actions */

	/* other static class parameters useful for debugging */
	int             priority;
	int             maxidle;
	int             minidle;
	int             offtime;
	int             qmax;
	int             ns_per_byte;
	int             wrr_allot;

	int             qcnt;           /* # packets in queue */
	int             avgidle;

	/* RED, RIO, BLUE, SFB related info */
	classq_type_t   qtype;
	union {
		/* RIO has 3 red stats */
		struct red_stats        red[RIO_NDROPPREC];
		struct blue_stats       blue;
		struct sfb_stats        sfb;
	};
	classq_state_t  qstate;
} class_stats_t;

#ifdef __cplusplus
}
#endif
#endif /* PRIVATE */
#endif /* !_NET_PKTSCHED_PKTSCHED_CBQ_H_ */
