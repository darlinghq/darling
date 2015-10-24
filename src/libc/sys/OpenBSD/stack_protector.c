/*	$OpenBSD: stack_protector.c,v 1.10 2006/03/31 05:34:44 deraadt Exp $	*/

/*
 * Copyright (c) 2002 Hiroaki Etoh, Federico G. Schwindt, and Miodrag Vallat.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <sys/param.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <asl.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "CrashReporterClient.h"
#include "libproc.h"
#include "_simple.h"

#define	GUARD_MAX 8
long __stack_chk_guard[GUARD_MAX] = {0, 0, 0, 0, 0, 0, 0, 0};
void __abort(void) __dead2;
void __guard_setup(const char *apple[]) __attribute__ ((visibility ("hidden")));
void __stack_chk_fail(void);

static void
__guard_from_kernel(const char *str)
{
	unsigned long long val;
	char tmp[20], *p;
	int idx = 0;

	/* Skip over the 'stack_guard=' key to the list of values */
	str = strchr(str, '=');
	if (str == NULL)
		return;
	str++;

	while (str && idx < GUARD_MAX) {
		/*
		 * Pull the next numeric string out of the list and convert it to
		 * a real number.
		 */
		strlcpy(tmp, str, 20);
		p = strchr(tmp, ',');
		if (p)
			*p = '\0';
		val = strtoull(tmp, NULL, 0);
		__stack_chk_guard[idx] = (long)(val & ((unsigned long) -1));
		idx++;
		if ((str = strchr(str, ',')) != NULL)
			str++;
	}
}

void
__guard_setup(const char *apple[])
{
	int fd;
	size_t len;
	const char **p;

	if (__stack_chk_guard[0] != 0)
		return;

	for (p = apple; p && *p; p++) {
		if (strstr(*p, "stack_guard") == *p) {
			__guard_from_kernel(*p);
			if (__stack_chk_guard[0] != 0)
				return;
		}
	}

	fd = open ("/dev/urandom", 0);
	if (fd != -1) {
		len = read (fd, (char*)&__stack_chk_guard, sizeof(__stack_chk_guard));
		close(fd);
		if (len == sizeof(__stack_chk_guard) && 
                    *__stack_chk_guard != 0)
			return;
	}

	/* If If a random generator can't be used, the protector switches the guard
           to the "terminator canary" */
	((unsigned char *)__stack_chk_guard)[0] = 0;
	((unsigned char *)__stack_chk_guard)[1] = 0;
	((unsigned char *)__stack_chk_guard)[2] = '\n';
	((unsigned char *)__stack_chk_guard)[3] = 255;
}

#define STACKOVERFLOW	"] stack overflow"

void
__stack_chk_fail()
{
	char n[16]; // bigger than will hold the digits in a pid_t
	char *np;
	int pid = getpid();
	char message[sizeof(n) + sizeof(STACKOVERFLOW)] = "[";
	char prog[2*MAXCOMLEN+1] = {0};

	proc_name(pid, prog, 2*MAXCOMLEN);
	prog[2*MAXCOMLEN] = 0;
	np = n + sizeof(n);
	*--np = 0;
	while(pid > 0) {
	    *--np = (pid % 10) + '0';
	    pid /= 10;
	}
	strlcat(message, np, sizeof(message));
	strlcat(message, STACKOVERFLOW, sizeof(message));
	/* This may fail on a chroot jail... */
	_simple_asl_log_prog(ASL_LEVEL_CRIT, "user", message, prog);

	CRSetCrashLogMessage(message);
	__abort();
}
