/*
 * Copyright (c) 1999, 2005 Apple Computer, Inc. All rights reserved.
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

#include <string.h>
#include <unistd.h>
#ifdef UTMP_COMPAT
#include <utmp.h>
#endif /* UTMP_COMPAT */
#include <utmpx.h>
#include <utmpx-darwin.h>

void
logwtmp(char *line, char *name, char *host)
{
	struct utmpx utx;
#ifdef UTMP_COMPAT
#ifdef __LP64__
	struct utmp32 u;
#else /* __LP64__ */
	struct utmp u;
#endif /* __LP64__ */
	int which;
#endif /* UTMP_COMPAT */

	bzero(&utx, sizeof(utx));
	/*
	 * line should never be "|" or "{", because this interface doesn't allow
	 * setting ut_tv.
	 */
	if (strcmp(line, "~") == 0)
		utx.ut_type = strcmp(name, "reboot") == 0 ? BOOT_TIME : SHUTDOWN_TIME;
	else {
		strncpy(utx.ut_user, name, sizeof(utx.ut_user));
		strncpy(utx.ut_line, line, sizeof(utx.ut_line));
		utx.ut_pid = getpid();
		utx.ut_type = *name ? USER_PROCESS : DEAD_PROCESS;
		strncpy(utx.ut_host, host, sizeof(utx.ut_host));
	}
	(void)gettimeofday(&utx.ut_tv, NULL);
	_utmpx_asl(&utx);
#ifdef UTMP_COMPAT
	which = _utmp_compat(&utx, &u);
	if (which & UTMP_COMPAT_WTMP)
		_write_wtmp(&u);
	if (which & UTMP_COMPAT_LASTLOG)
		_write_lastlog(&u, NULL);
#endif /* UTMP_COMPAT */
}
