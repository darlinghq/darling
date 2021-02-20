/*
 * Copyright (c) 1999-2018 Apple Inc. All rights reserved.
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

/*	Bertrand from vmutils -> CF -> System */

#include <pthread.h>
#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <stdlib.h>
#include <pthread/stack_np.h>
#include "stack_logging.h"

#define	INSTACK(a)	((a) >= stackbot && (a) <= stacktop)
#if defined(__x86_64__)
#define	ISALIGNED(a)	((((uintptr_t)(a)) & 0xf) == 0)
#elif defined(__i386__)
#define	ISALIGNED(a)	((((uintptr_t)(a)) & 0xf) == 8)
#elif defined(__arm__) || defined(__arm64__)
#define	ISALIGNED(a)	((((uintptr_t)(a)) & 0x1) == 0)
#endif

__attribute__((noinline))
static void
__thread_stack_pcs(vm_address_t *buffer, unsigned max, unsigned *nb,
		unsigned skip, void *startfp)
{
	void *frame, *next;
	pthread_t self = pthread_self();
	void *stacktop = pthread_get_stackaddr_np(self);
	void *stackbot = stacktop - pthread_get_stacksize_np(self);

	*nb = 0;

	// Rely on the fact that our caller has an empty stackframe (no local vars)
	// to determine the minimum size of a stackframe (frame ptr & return addr)
	frame = __builtin_frame_address(0);
	next = (void*)pthread_stack_frame_decode_np((uintptr_t)frame, NULL);

	/* make sure return address is never out of bounds */
	stacktop -= (next - frame);

	if(!INSTACK(frame) || !ISALIGNED(frame))
		return;
	while (startfp || skip--) {
		if (startfp && startfp < next) break;
		if(!INSTACK(next) || !ISALIGNED(next) || next <= frame)
			return;
		frame = next;
		next = (void*)pthread_stack_frame_decode_np((uintptr_t)frame, NULL);
	}
	while (max--) {
		uintptr_t retaddr;
		next = (void*)pthread_stack_frame_decode_np((uintptr_t)frame, &retaddr);
		buffer[*nb] = retaddr;
		(*nb)++;
		if(!INSTACK(next) || !ISALIGNED(next) || next <= frame)
			return;
		frame = next;
	}
}

// Note that callee relies on this function having a minimal stackframe
// to introspect (i.e. no tailcall and no local variables)
__private_extern__ __attribute__((disable_tail_calls))
void
_thread_stack_pcs(vm_address_t *buffer, unsigned max, unsigned *nb,
		unsigned skip, void *startfp)
{
	// skip this frame
	__thread_stack_pcs(buffer, max, nb, skip + 1, startfp);
}

// Prevent thread_stack_pcs() from getting tail-call-optimized into
// __thread_stack_pcs() on 64-bit environments, thus making the "number of hot
// frames to skip" be more predictable, giving more consistent backtraces.
//
// See <rdar://problem/5364825> "stack logging: frames keep getting truncated"
// for why this is necessary.
//
// Note that callee relies on this function having a minimal stackframe
// to introspect (i.e. no tailcall and no local variables)
__attribute__((disable_tail_calls))
void
thread_stack_pcs(vm_address_t *buffer, unsigned max, unsigned *nb)
{
	__thread_stack_pcs(buffer, max, nb, 0, NULL);
}
