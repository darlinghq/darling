/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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
#ifdef KERNEL
#ifndef _STRUCT_USER64_TIMEX
#define _STRUCT_USER64_TIMEX    struct user64_timex
_STRUCT_USER64_TIMEX
{
	u_int32_t modes;
	user64_long_t   offset;
	user64_long_t   freq;
	user64_long_t   maxerror;
	user64_long_t   esterror;
	__int32_t       status;
	user64_long_t   constant;
	user64_long_t   precision;
	user64_long_t   tolerance;

	user64_long_t   ppsfreq;
	user64_long_t   jitter;
	__int32_t       shift;
	user64_long_t   stabil;
	user64_long_t   jitcnt;
	user64_long_t   calcnt;
	user64_long_t   errcnt;
	user64_long_t   stbcnt;
};
#endif /* _STRUCT_USER64_TIMEX */
#endif /* KERNEL */
