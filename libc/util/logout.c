/*
 * Copyright (c) 1999, 2005, 2010 Apple Inc. All rights reserved.
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
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/time.h>

#ifdef UTMP_COMPAT
#include <utmp.h>
#endif /* UTMP_COMPAT */
#include <utmpx.h>
#include <utmpx-darwin.h>
#include <unistd.h>
#include <string.h>

int
logout(char *line)
{
	struct utmpx *ux, utx;
#ifdef UTMP_COMPAT
#ifdef __LP64__
	struct utmp32 u;
#else /* __LP64__ */
	struct utmp u;
#endif /* __LP64__ */
	int which;
#endif /* UTMP_COMPAT */

	bzero(&utx, sizeof(utx));
	strncpy(utx.ut_line, line, sizeof(utx.ut_line));
	utx.ut_type = UTMPX_AUTOFILL_MASK | UTMPX_DEAD_IF_CORRESPONDING_MASK | DEAD_PROCESS;
	(void)gettimeofday(&utx.ut_tv, NULL);
	UTMPX_LOCK(&__utx__);
	__setutxent(&__utx__);
	ux = __pututxline(&__utx__, &utx);
	__endutxent(&__utx__);
	if (!ux) {
		UTMPX_UNLOCK(&__utx__);
		return 0;
	}
#ifdef UTMP_COMPAT
	if (__utx__.utfile_system) { /* only if we are using _PATH_UTMPX */
		which = _utmp_compat(ux, &u);
		if (which & UTMP_COMPAT_UTMP0)
			_write_utmp(&u, 0);
		else if (which & UTMP_COMPAT_UTMP1)
			_write_utmp(&u, 1);
	}
#endif /* UTMP_COMPAT */
	UTMPX_UNLOCK(&__utx__);
	return 1;
}
