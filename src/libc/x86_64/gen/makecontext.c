/*
 * Copyright (c) 2007, 2009 Apple Inc. All rights reserved.
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
 * Copyright (c) 2001 Daniel M. Eischen <deischen@freebsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of the author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(__x86_64__)

#define _XOPEN_SOURCE
#include <sys/cdefs.h>

#include <sys/param.h>
#include <sys/signal.h>
#include <sys/ucontext.h>

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>

/* Prototypes */
extern void _ctx_start(ucontext_t *, int argc, ...);

void
_ctx_done (ucontext_t *ucp)
{
	if (ucp->uc_link == NULL)
		exit(0);
	else {
		/*
		 * Since this context has finished, don't allow it
		 * to be restarted without being reinitialized (via
		 * setcontext or swapcontext).
		 */
		ucp->uc_mcsize = 0;

		/* Set context to next one in link */
		/* XXX - what to do for error, abort? */
		setcontext((const ucontext_t *)ucp->uc_link);
		LIBC_ABORT("setcontext failed");	/* should never get here */
	}
}

void
makecontext(ucontext_t *ucp, void (*start)(), int argc, ...)
{
	va_list		ap;
	char		*stack_top;
	intptr_t	*argp;
	int		i;

	if (ucp == NULL)
		return;
	else if ((ucp->uc_stack.ss_sp == NULL) ||
	    (ucp->uc_stack.ss_size < MINSIGSTKSZ)) {
		/*
		 * This should really return -1 with errno set to ENOMEM
		 * or something, but the spec says that makecontext is
		 * a void function.   At least make sure that the context
		 * isn't valid so it can't be used without an error.
		 */
		ucp->uc_mcsize = 0;
	}
	/* XXX - Do we want to sanity check argc? */
	else if ((argc < 0) || (argc > NCARGS)) {
		ucp->uc_mcsize = 0;
	}
	/* Make sure the context is valid. */
	else {
		/*
		 * Arrange the stack as follows:
		 *
		 *	_ctx_start()	- context start wrapper
		 *	start()		- user start routine
		 * 	arg1            - first argument, aligned(16)
		 *	...
		 *	argn
		 *	ucp		- this context, %rbp points here
		 *
		 * When the context is started, control will return to
		 * the context start wrapper which will pop the user
		 * start routine from the top of the stack.  After that,
		 * the top of the stack will be setup with all arguments
		 * necessary for calling the start routine.  When the
		 * start routine returns, the context wrapper then sets
		 * the stack pointer to %rbp which was setup to point to
		 * the base of the stack (and where ucp is stored).  It
		 * will then call _ctx_done() to swap in the next context
		 * (uc_link != 0) or exit the program (uc_link == 0).
		 */
		mcontext_t mc;

		stack_top = (char *)(ucp->uc_stack.ss_sp +
		    ucp->uc_stack.ss_size - sizeof(intptr_t));


		/* Give 6 stack slots to _ctx_start */
		int minargc = 6;
		if (argc > minargc) 
			minargc = argc;

		/*
		 * Adjust top of stack to allow for 3 pointers (return
		 * address, _ctx_start, and ucp) and argc arguments.
		 * We allow the arguments to be pointers also.  The first
		 * argument to the user function must be properly aligned.
		 */

		stack_top = stack_top - (sizeof(intptr_t) * (1 + minargc));
		stack_top = (char *)((intptr_t)stack_top & ~15);
		stack_top = stack_top - (2 * sizeof(intptr_t));
		argp = (intptr_t *)stack_top;

		/*
		 * Setup the top of the stack with the user start routine
		 * followed by all of its aguments and the pointer to the
		 * ucontext.  We need to leave a spare spot at the top of
		 * the stack because setcontext will move rip to the top
		 * of the stack before returning.
		 */
		*argp = (intptr_t)_ctx_start;  /* overwritten with same value */
		argp++;
		*argp = (intptr_t)start;
		argp++;

		/* Add all the arguments: */
		va_start(ap, argc);
		for (i = 0; i < argc; i++) {
			*argp = va_arg(ap, intptr_t);
			argp++;
		}
		va_end(ap);

		/* Always provide space for ctx_start to pop the parameter registers */
		for (;argc < minargc; argc++) { 
			*argp++ = 0;
		}

		/* Keep stack aligned */
		if (argc & 1) {
			*argp++ = 0;
		}

		/* The ucontext is placed at the bottom of the stack. */
		*argp = (intptr_t)ucp;

		/*
		 * Set the machine context to point to the top of the
		 * stack and the program counter to the context start
		 * wrapper.  Note that setcontext() pushes the return
		 * address onto the top of the stack, so allow for this
		 * by adjusting the stack downward 1 slot.  Also set
		 * %r12 to point to the base of the stack where ucp
		 * is stored.
		 */
		mc = ucp->uc_mcontext;
		/* Use callee-save and match _ctx_start implementation */
		mc->__ss.__r12 = (intptr_t)argp;
		mc->__ss.__rbp = 0;
		mc->__ss.__rsp = (intptr_t)stack_top + sizeof(caddr_t);
		mc->__ss.__rip = (intptr_t)_ctx_start;
	}
}

#endif /* __x86_64__ */
