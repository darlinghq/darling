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

#if defined(LIBC_SCCS) && !defined(list)
static char rcsid[] = "$OpenBSD: stack_protector.c,v 1.3 2002/12/10 08:53:42 etoh Exp $";
#endif

#include <stdlib.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

long __stack_chk_guard[8] = {0, 0, 0, 0, 0, 0, 0, 0};
void __guard_setup(void) __attribute__ ((visibility ("hidden")));
void __stack_chk_fail(void);

void
__guard_setup(void)
{
  int fd;
  if (__stack_chk_guard[0]!=0) return;
  fd = open ("/dev/urandom", 0);
  if (fd != -1) {
    ssize_t size = read (fd, (char*)&__stack_chk_guard,
			 sizeof(__stack_chk_guard));
    close (fd) ;
    if (size == sizeof(__stack_chk_guard)
	&& *__stack_chk_guard != 0) return;
  }
  /* If a random generator can't be used, the protector switches the guard
     to the "terminator canary" */
  ((char*)__stack_chk_guard)[0] = 0; ((char*)__stack_chk_guard)[1] = 0;
  ((char*)__stack_chk_guard)[2] = '\n'; ((char*)__stack_chk_guard)[3] = 255;
}

void
__stack_chk_fail()
{
  const char message[] = "[%d] stack overflow";

  /* this may fail on a chroot jail, though luck */
  syslog(LOG_CRIT, message, getpid());

  abort();
}
