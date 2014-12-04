/*
 * Copyright (c) 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
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

#if defined(__i386__) || defined(__x86_64__)

#include <stdlib.h>
#include <machine/cpu_capabilities.h>
#include "platfunc.h"

#define PLAT_CAP_MASK (kHasMMX | kHasSSE | kHasSSE2 | kHasSSE3 | kCache32 | kCache64 | kCache128 | kFastThreadLocalStorage | kHasSupplementalSSE3 | k64Bit | kHasSSE4_1 | kHasSSE4_2 | kHasAES | kInOrderPipeline | kSlow | kUP)

inline static int __get_cpu_capabilities() {
	return *((int32_t*)_COMM_PAGE_CPU_CAPABILITIES);
}

inline static int num_cpus(int capabilities) {
	return (capabilities & kNumCPUs) >> kNumCPUsShift;
}

/* TODO: If 8151810 is fixed (or full support for visibility("internal") is added), change this to visibility("internal") */
void __attribute__((visibility("hidden"))) *find_platform_function(const platfunc_descriptor *descriptors[]) {
	int cap = __get_cpu_capabilities(),
	    have_cpus = num_cpus(cap);

	for (int i = 0; descriptors[i] != 0; i++) {
		int musthave_cpus = num_cpus(descriptors[i]->musthave),
		    canthave_cpus = num_cpus(descriptors[i]->canthave);
		uint32_t musthave = descriptors[i]->musthave & PLAT_CAP_MASK,
				 canthave = descriptors[i]->canthave & PLAT_CAP_MASK;
		/* First check that the CPU supports all the features that are in musthave */
		if ((musthave & cap) != musthave)
			continue;
		/* Now check that the CPU doesn't have any of the features that are in canthave */
		if ((canthave & cap) != 0)
			continue;
		/* Special case check for the number of CPUs. */
		/* TODO: Should there be a way to specify < or > num cpus? */
		if (musthave_cpus != 0 && have_cpus != musthave_cpus)
			continue;
		if (canthave_cpus != 0 && have_cpus == canthave_cpus)
			continue;
		/* otherwise, we have found our preferred implementation */
		return descriptors[i]->address;
	}

	/* We didn't find an acceptable implementation. (bug in data structures!) */
	return NULL;
}

#endif
