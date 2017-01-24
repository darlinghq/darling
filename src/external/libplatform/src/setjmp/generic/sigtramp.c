/*
 * Copyright (c) 1999, 2007 Apple Inc. All rights reserved.
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
 * Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved
 */

#define _XOPEN_SOURCE 600

#import	<sys/types.h>
#import	<signal.h>
#import	<unistd.h>
#import	<ucontext.h>
#import	<mach/thread_status.h>
#include <TargetConditionals.h>

extern int __sigreturn(ucontext_t *, int);

/*
 * sigvec registers _sigtramp as the handler for any signal requiring
 * user-mode intervention.  All _sigtramp does is find the real handler,
 * calls it, then sigreturn's.
 *
 * Note that the kernel saves/restores all of our register state.
 */

/* On i386, i386/sys/_sigtramp.s defines this. There is no in_sigtramp on arm */
#if defined(__DYNAMIC__) && defined(__x86_64__)
__attribute__((visibility("hidden")))
int __in_sigtramp = 0;
#endif

/* These defn should match the kernel one */
#define UC_TRAD			1
#define UC_FLAVOR		30
#if defined(__ppc__) || defined(__ppc64__)
#define UC_TRAD64		20
#define UC_TRAD64_VEC		25
#define UC_FLAVOR_VEC		35
#define UC_FLAVOR64		40
#define UC_FLAVOR64_VEC		45
#define UC_DUAL			50
#define UC_DUAL_VEC		55

 /* The following are valid mcontext sizes */
#define UC_FLAVOR_SIZE ((PPC_THREAD_STATE_COUNT + PPC_EXCEPTION_STATE_COUNT + PPC_FLOAT_STATE_COUNT) * sizeof(int))

#define UC_FLAVOR_VEC_SIZE ((PPC_THREAD_STATE_COUNT + PPC_EXCEPTION_STATE_COUNT + PPC_FLOAT_STATE_COUNT + PPC_VECTOR_STATE_COUNT) * sizeof(int))

#define UC_FLAVOR64_SIZE ((PPC_THREAD_STATE64_COUNT + PPC_EXCEPTION_STATE64_COUNT + PPC_FLOAT_STATE_COUNT) * sizeof(int))

#define UC_FLAVOR64_VEC_SIZE ((PPC_THREAD_STATE64_COUNT + PPC_EXCEPTION_STATE64_COUNT + PPC_FLOAT_STATE_COUNT + PPC_VECTOR_STATE_COUNT) * sizeof(int))
#endif

#define UC_SET_ALT_STACK	0x40000000
#define UC_RESET_ALT_STACK	0x80000000

/*
 * Reset the kernel's idea of the use of an alternate stack; this is used by
 * both longjmp() and siglongjmp().  Nothing other than this reset is needed,
 * since restoring the registers and other operations that would normally be
 * done by sigreturn() are handled in user space, so we do not pass a user
 * context (in PPC, a user context is not the same as a jmpbuf mcontext, due
 * to having more than one set of registers, etc., for the various 32/64 etc.
 * contexts)..
 */
void
_sigunaltstack(int set)
{
        /* sigreturn(uctx, ctxstyle); */
	/* syscall (SYS_SIGRETURN, uctx, ctxstyle); */
	__sigreturn (NULL, (set == SS_ONSTACK) ? UC_SET_ALT_STACK : UC_RESET_ALT_STACK);
}

/* On these architectures, _sigtramp is implemented in assembly to
   ensure it matches its DWARF unwind information.  */
#if !defined (__i386__) && !defined (__x86_64__)

void
_sigtramp(
	union __sigaction_u __sigaction_u,
	int 			sigstyle,
	int 			sig,
	siginfo_t		*sinfo,
	ucontext_t		*uctx
) {
	int ctxstyle = UC_FLAVOR;

	if (sigstyle == UC_TRAD)
		sa_handler(sig);
	else {
#if TARGET_OS_WATCH
		// <rdar://problem/22016014>
		sa_sigaction(sig, sinfo, NULL);
#else
		sa_sigaction(sig, sinfo, uctx);
#endif
	}

        /* sigreturn(uctx, ctxstyle); */
	/* syscall (SYS_SIGRETURN, uctx, ctxstyle); */
	__sigreturn (uctx, ctxstyle);
}

#endif /* not ppc nor ppc64 nor i386 nor x86_64 */
