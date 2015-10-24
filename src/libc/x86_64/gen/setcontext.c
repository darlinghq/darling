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

#if defined(__x86_64__)

#define _XOPEN_SOURCE 600L
#include <ucontext.h>
#undef _XOPEN_SOURCE
#undef _POSIX_C_SOURCE /* sigsetmask() */
#include <signal.h>

extern int _setcontext(const mcontext_t);

int
setcontext(const ucontext_t *uctx)
{
	mcontext_t mctx = (mcontext_t)&uctx->__mcontext_data;
	ucontext_t *_uctx = (ucontext_t *)uctx;
	if (mctx != _uctx->uc_mcontext)
		_uctx->uc_mcontext = mctx;
	sigsetmask(uctx->uc_sigmask);
	return _setcontext(mctx);
}

#endif /* __x86_64__ */
