/*
 * Copyright (c) 2000-2003, 2008 Apple Inc. All rights reserved.
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
/*
 * Copyright 1996 1995 by Open Software Foundation, Inc. 1997 1996 1995 1994 1993 1992 1991  
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 * 
 */
/*
 * MkLinux
 */

/*
 * Machine specific support for thread initialization
 */

#if defined(__ppc__) || defined(__ppc64__)
#include <architecture/ppc/cframe.h>
#elif defined(__arm__) 
#include <architecture/arm/cframe.h>
#endif

#include "pthread_internals.h"

/*
 * Set up the initial state of a MACH thread
 */
void
_pthread_setup(pthread_t thread, 
	       void (*routine)(pthread_t), 
	       void *vsp, int suspended,
	       int needresume)
{
        kern_return_t r;
        unsigned int count;
#if defined(__ppc__) || defined(__ppc64__)
#if defined(__ppc__)
        ppc_thread_state_t state = {0};
	ppc_thread_state_t *ts = &state;
	thread_state_flavor_t flavor = PPC_THREAD_STATE;
	count = PPC_THREAD_STATE_COUNT;
#elif defined(__ppc64__)
        ppc_thread_state64_t state = {0};
	ppc_thread_state64_t *ts = &state;
	thread_state_flavor_t flavor = PPC_THREAD_STATE64;
	count = PPC_THREAD_STATE64_COUNT;
#endif
	/*
	 * Set up PowerPC registers.
	 */
	if (suspended) {
		PTHREAD_MACH_CALL(thread_get_state(thread->kernel_thread,
					   flavor,
					   (thread_state_t) &state,
					   &count),
			  r);
	}
	ts->srr0 = (uintptr_t)routine;
        ts->r1 = (uintptr_t)vsp - C_ARGSAVE_LEN - C_RED_ZONE;
	ts->r3 = (uintptr_t)thread;
	/* Incase of needresume, suspend is always set */
	if (suspended) {
		PTHREAD_MACH_CALL(thread_set_state(thread->kernel_thread,
					   flavor,
					   (thread_state_t) &state,
					   count),
			  r);
		if (needresume)
			PTHREAD_MACH_CALL(thread_resume(thread->kernel_thread),
				r);
	} else {
		PTHREAD_MACH_CALL(thread_create_running(mach_task_self(),
					flavor,
					(thread_state_t) ts,
					count,
					&thread->kernel_thread),
			r);
	}
#elif defined(__i386__)
        i386_thread_state_t state = {0};
        i386_thread_state_t *ts = &state;
        int *sp = vsp;

        /*
         * Set up i386 registers & function call.
         */
        count = i386_THREAD_STATE_COUNT;
	if (suspended) {
	        PTHREAD_MACH_CALL(thread_get_state(thread->kernel_thread,
					   i386_THREAD_STATE,
					   (thread_state_t) &state,
					   &count),
			  r);
	}
        ts->eip = (int) routine;

        /*
        ** We need to simulate a 16-byte aligned stack frame as if we had
        ** executed a call instruction. Since we're "pushing" one argument,
        ** we need to adjust the pointer by 12 bytes (3 * sizeof (int *))
        */

        sp -= 3;              /* make sure stack is aligned */
        *--sp = (int) thread; /* argument to function */
        *--sp = 0;            /* fake return address */
        ts->esp = (int) sp;   /* set stack pointer */
	/* Incase of needresume, suspend is always set */
        if (suspended) {
		PTHREAD_MACH_CALL(thread_set_state(thread->kernel_thread,
					   i386_THREAD_STATE,
					   (thread_state_t) &state,
					   i386_THREAD_STATE_COUNT),
			  r);
		if (needresume)
			PTHREAD_MACH_CALL(thread_resume(thread->kernel_thread),
				r);
	} else {
		PTHREAD_MACH_CALL(thread_create_running(mach_task_self(),
					i386_THREAD_STATE,
					(thread_state_t) ts,
					i386_THREAD_STATE_COUNT,
					&thread->kernel_thread),
			r);
	}

#elif defined(__x86_64__)
        x86_thread_state64_t state = {0};
        x86_thread_state64_t *ts = &state;
        uintptr_t *sp = vsp;

        /*
         * Set up x86-64 registers & function call.
         */
        count = x86_THREAD_STATE64_COUNT;
	if (suspended) {
	        PTHREAD_MACH_CALL(thread_get_state(thread->kernel_thread,
					   x86_THREAD_STATE64,
					   (thread_state_t) &state,
					   &count),
			  r);
	}
        ts->rip = (uintptr_t) routine;

        /*
        ** We need to simulate a 16-byte aligned stack frame as if we had
        ** executed a call instruction. The stack should already be aligned
		** before it comes to us and we don't need to push any arguments,
		** so we shouldn't need to change it.
        */

		ts->rdi = (uintptr_t) thread;	/* argument to function */
        *--sp = 0;            /* fake return address */
        ts->rsp = (uintptr_t) sp;   /* set stack pointer */
	/* Incase of needresume, suspend is always set */
        if (suspended) {
		PTHREAD_MACH_CALL(thread_set_state(thread->kernel_thread,
					   x86_THREAD_STATE64,
					   (thread_state_t) &state,
					   x86_THREAD_STATE64_COUNT),
			  r);
		if (needresume)
			PTHREAD_MACH_CALL(thread_resume(thread->kernel_thread),
				r);
	} else {
		PTHREAD_MACH_CALL(thread_create_running(mach_task_self(),
					x86_THREAD_STATE64,
					(thread_state_t) ts,
					x86_THREAD_STATE64_COUNT,
					&thread->kernel_thread),
			r);
	}

#elif defined(__arm__)
	arm_thread_state_t state = {0};
	arm_thread_state_t *ts = &state;
	thread_state_flavor_t flavor = ARM_THREAD_STATE;
	count = ARM_THREAD_STATE_COUNT;

	if (suspended) {
		PTHREAD_MACH_CALL(thread_get_state(thread->kernel_thread,
					   flavor,
					   (thread_state_t) &state,
					   &count),
			  r);
	}

	ts->pc = (uintptr_t)routine;

	if (ts->pc & 1) {
	    ts->pc &= ~1;
	    ts->cpsr |= 0x20; /* PSR_THUMB */
	}
	
        ts->sp = (uintptr_t)vsp - C_ARGSAVE_LEN - C_RED_ZONE;
	ts->r[0] = (uintptr_t)thread;

	/* Incase of needresume, suspend is always set */
	if (suspended) {
		PTHREAD_MACH_CALL(thread_set_state(thread->kernel_thread,
					   flavor,
					   (thread_state_t) &state,
					   count),
			  r);
		if (needresume)
			PTHREAD_MACH_CALL(thread_resume(thread->kernel_thread),
				r);
	} else {
		PTHREAD_MACH_CALL(thread_create_running(mach_task_self(),
					flavor,
					(thread_state_t) ts,
					count,
					&thread->kernel_thread),
			r);
	}
#else
#error _pthread_setup not defined for this architecture
#endif
}
