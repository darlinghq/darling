/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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
 * @OSF_COPYRIGHT@
 */
#ifndef _ARM_CPU_INTERNAL_H_
#define _ARM_CPU_INTERNAL_H_


#include <mach/kern_return.h>
#include <arm/cpu_data_internal.h>

extern void                                             cpu_bootstrap(
	void);

extern void                                             cpu_init(
	void);

extern void                                             cpu_timebase_init(boolean_t from_boot);

extern kern_return_t                    cpu_signal(
	cpu_data_t              *target,
	unsigned int    signal,
	void                    *p0,
	void                    *p1);

extern kern_return_t                    cpu_signal_deferred(
	cpu_data_t              *target);

extern void                     cpu_signal_cancel(
	cpu_data_t              *target);

#define SIGPnop                 0x00000000U             /* Send IPI with no service */
#define SIGPdec                 0x00000001U             /* Request decremeter service */
/* 0x2U unused */
#define SIGPxcall               0x00000004U             /* Call a function on a processor */
#define SIGPast                 0x00000008U             /* Request AST check */
#define SIGPdebug               0x00000010U             /* Request Debug call */
#define SIGPLWFlush             0x00000020UL            /* Request LWFlush call */
#define SIGPLWClean             0x00000040UL            /* Request LWClean call */
#define SIGPkptimer             0x00000100U             /* Request kperf timer */
#define SIGPxcallImm            0x00000200U             /* Send a cross-call, fail if already pending */

#define SIGPdisabled            0x80000000U             /* Signal disabled */

extern unsigned int real_ncpus;

#if defined(CONFIG_XNUPOST) && __arm64__
extern void arm64_ipi_test(void);
#endif /* defined(CONFIG_XNUPOST) && __arm64__ */


#endif  /* _ARM_CPU_INTERNAL_H_ */
