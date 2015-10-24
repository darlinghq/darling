/*
 * Copyright (c) 2007, 2008, 2009 Apple Inc. All rights reserved.
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

#if defined(__x86_64__)

#define _XOPEN_SOURCE 600L

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <ucontext.h>

extern size_t pthread_get_stacksize_np(pthread_t);
extern void *pthread_get_stackaddr_np(pthread_t);
#ifdef __DYNAMIC__
extern int __in_sigtramp;
#endif /* __DYNAMIC_ */

__private_extern__ mcontext_t
getmcontext(ucontext_t *uctx, void *sp)
{
	pthread_t self = pthread_self();
	mcontext_t mctx = (mcontext_t)&uctx->__mcontext_data;
	size_t stacksize = pthread_get_stacksize_np(self);
	stack_t stack;

	uctx->uc_stack.ss_sp = sp;
	uctx->uc_stack.ss_flags = 0;

	if (0 == sigaltstack(NULL, &stack)) {
		if (stack.ss_flags & SS_ONSTACK) {
			uctx->uc_stack = stack;
			stacksize = stack.ss_size;
		}
	}

	if (stacksize == 0) {	/* main thread doesn't have pthread stack size */
		struct rlimit rlim;
		if (0 == getrlimit(RLIMIT_STACK, &rlim))
			stacksize = rlim.rlim_cur;
	}

	uctx->uc_stack.ss_size = stacksize;

	if (uctx->uc_mcontext != mctx) {
		uctx->uc_mcontext = mctx;

#ifdef __DYNAMIC__
		uctx->uc_link = (ucontext_t*)(uintptr_t)__in_sigtramp; /* non-zero if in signal handler */
#else  /* !__DYNAMIC__ */
		uctx->uc_link = 0;
#endif /* __DYNAMIC__ */

	}

	sigprocmask(0, NULL, &uctx->uc_sigmask);
	return mctx;
}

#endif /* __x86_64__ */
