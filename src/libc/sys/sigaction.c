/*
 * Copyright (c) 1999-2017 Apple Computer, Inc. All rights reserved.
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

#if __has_include(<CrashReporterClient.h>)
#include <CrashReporterClient.h>
#else
#define CRSetCrashLogMessage(...)
#endif
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/signal.h>
#include <errno.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstrict-prototypes"

extern int __platform_sigaction (int sig,
		const struct sigaction * __restrict nsv,
		struct sigaction * __restrict osv);

int
sigaction (int sig, const struct sigaction * __restrict nsv,
		struct sigaction * __restrict osv)
{
	int ret = __platform_sigaction(sig, nsv, osv);
#ifdef FEATURE_SIGNAL_RESTRICTION
	// Note: The "sig != 0" here is to force the compiler to maintain that "sig"
	// is live, and in a register, after __sigaction so it is visible in the
	// crashing register state.
	if (ret == -1 && errno == ENOTSUP && sig != 0) {
		CRSetCrashLogMessage("sigaction on fatal signals is not supported");
		__builtin_trap();
	}
#endif
	return ret;
}

// XXX
#ifdef __DYNAMIC__

int
_sigaction_nobind (sig, nsv, osv)
        int sig;
	register const struct sigaction *nsv;
        register struct sigaction *osv;
{
    return sigaction(sig, nsv, osv);
}
#endif

#pragma clang diagnostic pop
