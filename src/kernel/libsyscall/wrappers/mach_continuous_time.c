/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>
#include <machine/cpu_capabilities.h>
#include <mach/mach_time.h>

__attribute__((visibility("hidden")))
uint64_t
_mach_continuous_time_base(void)
{
#if !defined(__x86_64__) && !defined(__arm64__)
	// Deal with the lack of 64-bit loads on arm32 (see mach_approximate_time.s)
	while (1) {
		volatile uint64_t *base_ptr = (volatile uint64_t*)_COMM_PAGE_CONT_TIMEBASE;
		uint64_t read1, read2;
		read1 = *base_ptr;
#if defined(__arm__)
		__asm__ volatile ("dsb sy" ::: "memory");
#elif defined(__i386__)
		__asm__ volatile ("lfence" ::: "memory");
#else
#error "unsupported arch"
#endif
		read2 = *base_ptr;

		if (__builtin_expect((read1 == read2), 1)) {
			return read1;
		}
	}
#else // 64-bit
	return *(volatile uint64_t*)_COMM_PAGE_CONT_TIMEBASE;
#endif // 64-bit
}

__attribute__((visibility("hidden")))
kern_return_t
_mach_continuous_hwclock(uint64_t *cont_time __unused)
{
#if defined(__arm64__)
#define ISB_SY          0xf
	uint8_t cont_hwclock = *((uint8_t*)_COMM_PAGE_CONT_HWCLOCK);
	if (cont_hwclock) {
		volatile uint64_t *base_ptr = (volatile uint64_t*)_COMM_PAGE_CONT_HW_TIMEBASE;
		__builtin_arm_isb(ISB_SY);
		*cont_time = __builtin_arm_rsr64("CNTVCT_EL0") + *base_ptr;
		return KERN_SUCCESS;
	}
#endif
	return KERN_NOT_SUPPORTED;
}

__attribute__((visibility("hidden")))
kern_return_t
_mach_continuous_time(uint64_t* absolute_time, uint64_t* cont_time)
{
	volatile uint64_t *base_ptr = (volatile uint64_t*)_COMM_PAGE_CONT_TIMEBASE;
	volatile uint64_t read1, read2;
	volatile uint64_t absolute;

	do {
		read1 = *base_ptr;
		absolute = mach_absolute_time();
#if     defined(__arm__) || defined(__arm64__)
		/*
		 * mach_absolute_time() contains an instruction barrier which will
		 * prevent the speculation of read2 above this point, so we don't
		 * need another barrier here.
		 */
#endif
		read2 = *base_ptr;
	} while (__builtin_expect((read1 != read2), 0));

	if (absolute_time) {
		*absolute_time = absolute;
	}
	if (cont_time) {
		*cont_time = absolute + read1;
	}

	return KERN_SUCCESS;
}

uint64_t
mach_continuous_time(void)
{
	uint64_t cont_time;
	if (_mach_continuous_hwclock(&cont_time) != KERN_SUCCESS) {
		_mach_continuous_time(NULL, &cont_time);
	}
	return cont_time;
}

uint64_t
mach_continuous_approximate_time(void)
{
	/*
	 * No retry loop here because if we use a slightly too old timebase that's
	 * okay, we are approximate time anyway.
	 */
	volatile register uint64_t time_base = _mach_continuous_time_base();
	return time_base + mach_approximate_time();
}
