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


#define _XOPEN_SOURCE 600L
#include <ucontext.h>
#include <errno.h>

#if defined(__x86_64__) || defined(__i386__)

#include <stddef.h>
#include <signal.h>

extern int _setcontext(const void *);

int
setcontext(const ucontext_t *uctx)
{
	mcontext_t mctx = (mcontext_t)&uctx->__mcontext_data;
	ucontext_t *_uctx = (ucontext_t *)uctx;
	if (mctx != _uctx->uc_mcontext)
		_uctx->uc_mcontext = mctx;
	sigprocmask(SIG_SETMASK, &uctx->uc_sigmask, NULL);

#if defined(__x86_64__)
	return _setcontext(mctx);
#else
	return _setcontext(uctx);
#endif
}

#else

int
setcontext(const ucontext_t *uctx)
{
	errno = ENOTSUP;
	return -1;
}

#endif
