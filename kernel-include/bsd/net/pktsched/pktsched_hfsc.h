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

/*	$NetBSD: altq_hfsc.h,v 1.8 2006/10/12 19:59:08 peter Exp $	*/
/*	$KAME: altq_hfsc.h,v 1.12 2003/12/05 05:40:46 kjc Exp $	*/

/*
 * Copyright (c) 1997-1999 Carnegie Mellon University. All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation is hereby granted (including for commercial or
 * for-profit use), provided that both the copyright notice and this
 * permission notice appear in all copies of the software, derivative
 * works, or modified versions, and any portions thereof.
 *
 * THIS SOFTWARE IS EXPERIMENTAL AND IS KNOWN TO HAVE BUGS, SOME OF
 * WHICH MAY HAVE SERIOUS CONSEQUENCES.  CARNEGIE MELLON PROVIDES THIS
 * SOFTWARE IN ITS ``AS IS'' CONDITION, AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * Carnegie Mellon encourages (but does not require) users of this
 * software to return any improvements or extensions that they make,
 * and to grant Carnegie Mellon the rights to redistribute these
 * changes without encumbrance.
 */
#ifndef _NET_PKTSCHED_PKTSCHED_HFSC_H_
#define _NET_PKTSCHED_PKTSCHED_HFSC_H_

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

struct service_curve {
	u_int32_t fl;   /* service curve flags */
	u_int64_t m1;   /* slope of the first segment in bits/sec */
	u_int32_t d;    /* the x-projection of the first segment in msec */
	u_int64_t m2;   /* slope of the second segment in bits/sec */
};

/* valid values for service curve flags */
#define HFSCF_M1_PCT            0x1     /* m1 is in percentage */
#define HFSCF_M2_PCT            0x10    /* m2 is in percentage */

#define HFSCF_USERFLAGS         (HFSCF_M1_PCT | HFSCF_M2_PCT)

/* special class handles */
#define HFSC_NULLCLASS_HANDLE   0
#define HFSC_MAX_CLASSES        64

/* hfsc class flags */
#define HFCF_RED                0x0001  /* use RED */
#define HFCF_ECN                0x0002  /* use ECN with RED/BLUE/SFB */
#define HFCF_RIO                0x0004  /* use RIO */
#define HFCF_CLEARDSCP          0x0010  /* clear diffserv codepoint */
#define HFCF_BLUE               0x0100  /* use BLUE */
#define HFCF_SFB                0x0200  /* use SFB */
#define HFCF_FLOWCTL            0x0400  /* enable flow control advisories */
#define HFCF_DEFAULTCLASS       0x1000  /* default class */
#ifdef BSD_KERNEL_PRIVATE
#define HFCF_RSC                0x10000 /* has realtime sc */
#define HFCF_FSC                0x20000 /* has linkshare sc */
#define HFCF_USC                0x40000 /* has upperlimit sc */
#define HFCF_LAZY               0x10000000 /* on-demand resource allocation */
#endif /* BSD_KERNEL_PRIVATE */

#define HFCF_USERFLAGS                                                  \
	(HFCF_RED | HFCF_ECN | HFCF_RIO | HFCF_CLEARDSCP | HFCF_BLUE |  \
	HFCF_SFB | HFCF_FLOWCTL | HFCF_DEFAULTCLASS)

#ifdef BSD_KERNEL_PRIVATE
#define HFCF_BITS \
	"\020\1RED\2ECN\3RIO\5CLEARDSCP\11BLUE\12SFB\13FLOWCTL\15DEFAULT" \
	"\21RSC\22FSC\23USC\35LAZY"
#else
#define HFCF_BITS \
	"\020\1RED\2ECN\3RIO\5CLEARDSCP\11BLUE\12SFB\13FLOWCTL\15DEFAULT"
#endif /* !BSD_KERNEL_PRIVATE */

/* service curve types */
#define HFSC_REALTIMESC         1
#define HFSC_LINKSHARINGSC      2
#define HFSC_UPPERLIMITSC       4
#define HFSC_DEFAULTSC          (HFSC_REALTIMESC|HFSC_LINKSHARINGSC)

struct hfsc_classstats {
	u_int32_t               class_id;
	u_int32_t               class_handle;
	struct service_curve    rsc;
	struct service_curve    fsc;
	struct service_curve    usc;    /* upper limit service curve */

	u_int64_t               total;  /* total work in bytes */
	u_int64_t               cumul;  /* cumulative work in bytes */
	                                /*   done by real-time criteria */
	u_int64_t               d;              /* deadline */
	u_int64_t               e;              /* eligible time */
	u_int64_t               vt;             /* virtual time */
	u_int64_t               f;              /* fit time for upper-limit */

	/* info helpful for debugging */
	u_int64_t               initvt;         /* init virtual time */
	u_int64_t               vtoff;          /* cl_vt_ipoff */
	u_int64_t               cvtmax;         /* cl_maxvt */
	u_int64_t               myf;            /* cl_myf */
	u_int64_t               cfmin;          /* cl_mincf */
	u_int64_t               cvtmin;         /* cl_mincvt */
	u_int64_t               myfadj;         /* cl_myfadj */
	u_int64_t               vtadj;          /* cl_vtadj */
	u_int64_t               cur_time;
	u_int32_t               machclk_freq;

	u_int32_t               qlength;
	u_int32_t               qlimit;
	struct pktcntr          xmit_cnt;
	struct pktcntr          drop_cnt;
	u_int32_t               period;

	u_int32_t               vtperiod;       /* vt period sequence no */
	u_int32_t               parentperiod;   /* parent's vt period seqno */
	int                     nactive;        /* number of active children */

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

#ifdef __cplusplus
}
#endif
#endif /* PRIVATE */
#endif /* _NET_PKTSCHED_PKTSCHED_HFSC_H_ */
