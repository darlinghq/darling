/*-
 * Copyright (c) 1992, 1993
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)assert.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/assert.c,v 1.8 2007/01/09 00:27:53 imp Exp $");

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include "CrashReporterClient.h"
#include "_simple.h"

void
__assert_rtn(func, file, line, failedexpr)
	const char *func, *file;
	int line;
	const char *failedexpr;
{
	if (func == (const char *)-1L) {
		/* 8462256: special case to replace __eprintf */
		_simple_dprintf(STDERR_FILENO,
		     "%s:%u: failed assertion `%s'\n", file, line, failedexpr);
		if (!CRGetCrashLogMessage()) {
			_SIMPLE_STRING s = _simple_salloc();
			if (s) {
				_simple_sprintf(s,
				  "%s:%u: failed assertion `%s'\n",
				  file, line, failedexpr);
				CRSetCrashLogMessage(_simple_string(s));
			} else
				CRSetCrashLogMessage(failedexpr);
		}
	} else if (func == NULL) {
		_simple_dprintf(STDERR_FILENO,
		     "Assertion failed: (%s), file %s, line %d.\n", failedexpr,
		     file, line);
		if (!CRGetCrashLogMessage()) {
			_SIMPLE_STRING s = _simple_salloc();
			if (s) {
				_simple_sprintf(s,
				  "Assertion failed: (%s), file %s, line %d.\n",
				  failedexpr, file, line);
				CRSetCrashLogMessage(_simple_string(s));
			} else
				CRSetCrashLogMessage(failedexpr);
		}
	} else {
		_simple_dprintf(STDERR_FILENO,
		     "Assertion failed: (%s), function %s, file %s, line %d.\n",
		     failedexpr, func, file, line);
		if (!CRGetCrashLogMessage()) {
			_SIMPLE_STRING s = _simple_salloc();
			if (s) {
				_simple_sprintf(s,
				  "Assertion failed: (%s), function %s, file %s, line %d.\n",
				  failedexpr, func, file, line);
				CRSetCrashLogMessage(_simple_string(s));
			} else
				CRSetCrashLogMessage(failedexpr);
		}
	}
	abort();
	/* NOTREACHED */
}
