/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <sys/time.h>
#include <mach/mach_time.h>
#include <machine/cpu_capabilities.h>

int __commpage_gettimeofday(struct timeval *);

__attribute__((visibility("hidden")))
int __commpage_gettimeofday_internal(struct timeval *tp, uint64_t *tbr_out);

#if   defined(__x86_64__) || defined(__i386__)

// XXX: must be kept in sync with __commpage_gettimeofday.s
int
__commpage_gettimeofday_internal(struct timeval *tp, uint64_t *tbr_out)
{
	volatile uint32_t *gtod_generation_p = _COMM_PAGE_GTOD_GENERATION;
	volatile uint64_t *gtod_sec_base_p = _COMM_PAGE_GTOD_SEC_BASE;
	volatile uint64_t *gtod_ns_base_p = _COMM_PAGE_GTOD_NS_BASE;

	uint64_t tbr, gen, tod_secs, tod_nsecs, elapsed;
	while(1) {
		gen = *gtod_generation_p;
		tbr = mach_absolute_time();
		tod_secs = *gtod_sec_base_p;
		tod_nsecs = *gtod_ns_base_p;
		uint64_t gen2 = *gtod_generation_p;
		if(__builtin_expect(gen, gen2) == gen2)
			break;
	}
	if (gen == 0) return KERN_FAILURE;
	elapsed = tbr - tod_nsecs;

	unsigned long secs;
	uint32_t nsec;
#if defined(__x86_64__)
	secs = elapsed/NSEC_PER_SEC;
	nsec = elapsed % NSEC_PER_SEC;
#elif defined(__i386__)
	uint32_t secs1, secs2;
	secs1 = elapsed >> 32;
	secs2 = elapsed;
	__asm__ (
	  "divl %4"
	  : "=a" (secs), "=d" (nsec)
	  : "0" (secs2), "1" (secs1), "rm" (NSEC_PER_SEC)
	);
#endif /* __i386 or __x86_64__ */
	tp->tv_sec = tod_secs + secs;
	tp->tv_usec = nsec / NSEC_PER_USEC;

	if (tbr_out) *tbr_out = tbr;

	return KERN_SUCCESS;
}

#endif
