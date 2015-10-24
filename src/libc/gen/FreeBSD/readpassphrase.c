/*	$OpenBSD: readpassphrase.c,v 1.12 2001/12/15 05:41:00 millert Exp $	*/

/*
 * Copyright (c) 2000 Todd C. Miller <Todd.Miller@courtesan.com>
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
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$OpenBSD: readpassphrase.c,v 1.12 2001/12/15 05:41:00 millert Exp $";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/readpassphrase.c,v 1.6 2002/03/09 03:16:41 green Exp $");

#include "xlocale_private.h"

#include "namespace.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <pwd.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <readpassphrase.h>
#include "un-namespace.h"

static volatile sig_atomic_t signo;

static void handler(int);

char *
readpassphrase(const char *prompt, char *buf, size_t bufsiz, int flags)
{
	ssize_t nr;
	int input, output, save_errno;
	char ch, *p, *end;
	struct termios term, oterm;
	struct sigaction sa, saveint, savehup, savequit, saveterm;
	struct sigaction savetstp, savettin, savettou;
	locale_t loc = __current_locale();

	/* I suppose we could alloc on demand in this case (XXX). */
	if (bufsiz == 0) {
		errno = EINVAL;
		return(NULL);
	}

restart:
	/*
	 * Read and write to /dev/tty if available.  If not, read from
	 * stdin and write to stderr unless a tty is required.
	 */
	if ((input = output = _open(_PATH_TTY, O_RDWR)) == -1) {
		if (flags & RPP_REQUIRE_TTY) {
			errno = ENOTTY;
			return(NULL);
		}
		input = STDIN_FILENO;
		output = STDERR_FILENO;
	}

	/*
	 * Catch signals that would otherwise cause the user to end
	 * up with echo turned off in the shell.  Don't worry about
	 * things like SIGALRM and SIGPIPE for now.
	 */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;		/* don't restart system calls */
	sa.sa_handler = handler;
	(void)_sigaction(SIGINT, &sa, &saveint);
	(void)_sigaction(SIGHUP, &sa, &savehup);
	(void)_sigaction(SIGQUIT, &sa, &savequit);
	(void)_sigaction(SIGTERM, &sa, &saveterm);
	(void)_sigaction(SIGTSTP, &sa, &savetstp);
	(void)_sigaction(SIGTTIN, &sa, &savettin);
	(void)_sigaction(SIGTTOU, &sa, &savettou);

	/* Turn off echo if possible. */
	if (tcgetattr(input, &oterm) == 0) {
		memcpy(&term, &oterm, sizeof(term));
		if (!(flags & RPP_ECHO_ON))
			term.c_lflag &= ~(ECHO | ECHONL);
		if (term.c_cc[VSTATUS] != _POSIX_VDISABLE)
			term.c_cc[VSTATUS] = _POSIX_VDISABLE;
		(void)tcsetattr(input, TCSAFLUSH|TCSASOFT, &term);
	} else {
		memset(&term, 0, sizeof(term));
		memset(&oterm, 0, sizeof(oterm));
	}

	(void)_write(output, prompt, strlen(prompt));
	end = buf + bufsiz - 1;
	for (p = buf; (nr = _read(input, &ch, 1)) == 1 && ch != '\n' && ch != '\r';) {
		if (p < end) {
			if ((flags & RPP_SEVENBIT))
				ch &= 0x7f;
			if (isalpha_l(ch, loc)) {
				if ((flags & RPP_FORCELOWER))
					ch = tolower_l(ch, loc);
				if ((flags & RPP_FORCEUPPER))
					ch = toupper_l(ch, loc);
			}
			*p++ = ch;
		}
	}
	*p = '\0';
	save_errno = errno;
	if (!(term.c_lflag & ECHO))
		(void)_write(output, "\n", 1);

	/* Restore old terminal settings and signals. */
	if (memcmp(&term, &oterm, sizeof(term)) != 0)
		(void)tcsetattr(input, TCSANOW|TCSASOFT, &oterm);
	(void)_sigaction(SIGINT, &saveint, NULL);
	(void)_sigaction(SIGHUP, &savehup, NULL);
	(void)_sigaction(SIGQUIT, &savequit, NULL);
	(void)_sigaction(SIGTERM, &saveterm, NULL);
	(void)_sigaction(SIGTSTP, &savetstp, NULL);
	(void)_sigaction(SIGTTIN, &savettin, NULL);
	(void)_sigaction(SIGTTOU, &savettou, NULL);
	if (input != STDIN_FILENO)
		(void)_close(input);

	/*
	 * If we were interrupted by a signal, resend it to ourselves
	 * now that we have restored the signal handlers.
	 */
	if (signo) {
		kill(getpid(), signo); 
		switch (signo) {
		case SIGTSTP:
		case SIGTTIN:
		case SIGTTOU:
			signo = 0;
			goto restart;
		}
	}

	errno = save_errno;
	return(nr == -1 ? NULL : buf);
}

char *
getpass(const char *prompt)
{
	static char buf[_PASSWORD_LEN + 1];

	if (readpassphrase(prompt, buf, sizeof(buf), RPP_ECHO_OFF) == NULL)
		buf[0] = '\0';
	return(buf);
}

static void handler(int s)
{

	signo = s;
}
