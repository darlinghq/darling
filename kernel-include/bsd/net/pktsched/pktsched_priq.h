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

/*	$NetBSD: altq_priq.h,v 1.7 2006/10/12 19:59:08 peter Exp $	*/
/*	$KAME: altq_priq.h,v 1.7 2003/10/03 05:05:15 kjc Exp $	*/
/*
 * Copyright (C) 2000-2003
 *	Sony Computer Science Laboratories Inc.  All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY SONY CSL AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL SONY CSL OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _NET_PKTSCHED_PKTSCHED_PRIQ_H_
#define _NET_PKTSCHED_PKTSCHED_PRIQ_H_

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

#define PRIQ_MAXPRI     16      /* upper limit of the number of priorities */

/* priq class flags */
#define PRCF_RED                0x0001  /* use RED */
#define PRCF_ECN                0x0002  /* use ECN with RED/BLUE/SFB */
#define PRCF_RIO                0x0004  /* use RIO */
#define PRCF_CLEARDSCP          0x0010  /* clear diffserv codepoint */
#define PRCF_BLUE               0x0100  /* use BLUE */
#define PRCF_SFB                0x0200  /* use SFB */
#define PRCF_FLOWCTL            0x0400  /* enable flow control advisories */
#define PRCF_DEFAULTCLASS       0x1000  /* default class */
#ifdef BSD_KERNEL_PRIVATE
#define PRCF_LAZY               0x10000000 /* on-demand resource allocation */
#endif /* BSD_KERNEL_PRIVATE */

#define PRCF_USERFLAGS                                                  \
	(PRCF_RED | PRCF_ECN | PRCF_RIO | PRCF_CLEARDSCP | PRCF_BLUE |  \
	PRCF_SFB | PRCF_FLOWCTL | PRCF_DEFAULTCLASS)

#ifdef BSD_KERNEL_PRIVATE
#define PRCF_BITS \
	"\020\1RED\2ECN\3RIO\5CLEARDSCP\11BLUE\12SFB\13FLOWCTL\15DEFAULT" \
	"\35LAZY"
#else
#define PRCF_BITS \
	"\020\1RED\2ECN\3RIO\5CLEARDSCP\11BLUE\12SFB\13FLOWCTL\15DEFAULT"
#endif /* !BSD_KERNEL_PRIVATE */

struct priq_classstats {
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

#ifdef __cplusplus
}
#endif
#endif /* PRIVATE */
#endif /* _NET_PKTSCHED_PKTSCHED_PRIQ_H_ */
