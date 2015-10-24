/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1993
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
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <netdb.h>

#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include <crt_externs.h>
 
#ifdef BUILDING_VARIANT
__private_extern__ int	_sl_LogFile;		/* fd for log */
__private_extern__ int	_sl_connected;		/* have done connect */
__private_extern__ int	_sl_LogStat;		/* status bits, set by openlog() */
__private_extern__ const char *_sl_LogTag;	/* string to tag the entry with */
__private_extern__ int	_sl_LogFacility;	/* default facility code */
__private_extern__ int	_sl_LogMask;		/* mask of priorities to be logged */
#else /* !BUILDING_VARIANT */
__private_extern__ int	_sl_LogFile = -1;		/* fd for log */
__private_extern__ int	_sl_connected = 0;		/* have done connect */
__private_extern__ int	_sl_LogStat = 0;		/* status bits, set by openlog() */
__private_extern__ const char *_sl_LogTag = NULL;	/* string to tag the entry with */
__private_extern__ int	_sl_LogFacility = LOG_USER;	/* default facility code */
__private_extern__ int	_sl_LogMask = 0xff;		/* mask of priorities to be logged */
#endif /* BUILDING_VARIANT */

/*
 * syslog, vsyslog --
 *	print message on log file; output is intended for syslogd(8).
 */
void
#ifdef __STDC__
syslog(int pri, const char *fmt, ...)
#else
syslog(pri, fmt, va_alist)
	int pri;
	char *fmt;
	va_dcl
#endif
{
	va_list ap;

#ifdef __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	vsyslog(pri, fmt, ap);
	va_end(ap);
}

void
vsyslog(pri, fmt, ap)
	int pri;
	register const char *fmt;
	va_list ap;
{
	register int cnt;
	register char ch, *p, *t;
	time_t now;
	int fd, saved_errno;
#define	TBUF_LEN	2048
#define	FMT_LEN		1024
	char *stdp, tbuf[TBUF_LEN], fmt_cpy[FMT_LEN];
	int tbuf_left, fmt_left, prlen;

#define	INTERNALLOG	LOG_ERR|LOG_CONS|LOG_PERROR|LOG_PID
	/* Check for invalid bits. */
	if (pri & ~(LOG_PRIMASK|LOG_FACMASK)) {
		syslog(INTERNALLOG,
		    "syslog: unknown facility/priority: %x", pri);
		pri &= LOG_PRIMASK|LOG_FACMASK;
	}

	/* Check priority against setlogmask values. */
	if (!(LOG_MASK(LOG_PRI(pri)) & _sl_LogMask))
		return;

	saved_errno = errno;

	/* Set default facility if none specified. */
	if ((pri & LOG_FACMASK) == 0)
		pri |= _sl_LogFacility;

	/* Build the message. */
	
	/*
 	 * Although it's tempting, we can't ignore the possibility of
	 * overflowing the buffer when assembling the "fixed" portion
	 * of the message.  Strftime's "%h" directive expands to the
	 * locale's abbreviated month name, but if the user has the
	 * ability to construct to his own locale files, it may be
	 * arbitrarily long.
	 */
	(void)time(&now);

	p = tbuf;  
	tbuf_left = TBUF_LEN;
	
#define	DEC()	\
	do {					\
		if (prlen >= tbuf_left)		\
			prlen = tbuf_left - 1;	\
		p += prlen;			\
		tbuf_left -= prlen;		\
	} while (0)

	prlen = snprintf(p, tbuf_left, "<%d>", pri);
	DEC();

	prlen = strftime(p, tbuf_left, "%h %e %T ", localtime(&now));
	DEC();

	if (_sl_LogStat & LOG_PERROR)
		stdp = p;
	if (_sl_LogTag == NULL)
		_sl_LogTag = *(*_NSGetArgv());
	if (_sl_LogTag != NULL) {
		prlen = snprintf(p, tbuf_left, "%s", _sl_LogTag);
		DEC();
	}
	if (_sl_LogStat & LOG_PID) {
		prlen = snprintf(p, tbuf_left, "[%d]", getpid());
		DEC();
	}
	if (_sl_LogTag != NULL) {
		if (tbuf_left > 1) {
			*p++ = ':';
			tbuf_left--;
		}
		if (tbuf_left > 1) {
			*p++ = ' ';
			tbuf_left--;
		}
	}

	/* 
	 * We wouldn't need this mess if printf handled %m, or if 
	 * strerror() had been invented before syslog().
	 */
	for (t = fmt_cpy, fmt_left = FMT_LEN; (ch = *fmt); ++fmt) {
		if (ch == '%' && fmt[1] == 'm') {
			++fmt;
			prlen = snprintf(t, fmt_left, "%s",
			    strerror(saved_errno));
			if (prlen >= fmt_left)
				prlen = fmt_left - 1;
			t += prlen;
			fmt_left -= prlen;
		} else {
			if (fmt_left > 1) {
				*t++ = ch;
				fmt_left--;
			}
		}
	}
	*t = '\0';

	prlen = vsnprintf(p, tbuf_left, fmt_cpy, ap);
	DEC();
	cnt = p - tbuf;

	/* Output to stderr if requested. */
	if (_sl_LogStat & LOG_PERROR) {
		struct iovec iov[2];

		iov[0].iov_base = stdp;
		iov[0].iov_len = cnt - (stdp - tbuf);
		iov[1].iov_base = "\n";
		iov[1].iov_len = 1;
		(void)writev(STDERR_FILENO, iov, 2);
	}

	/* Get connected, output the message to the local logger. */
	if (!_sl_connected)
		openlog(_sl_LogTag, _sl_LogStat | LOG_NDELAY, 0);
	if (send(_sl_LogFile, tbuf, cnt, 0) >= 0)
		return;

	/*
	 * Output the message to the console; don't worry about blocking,
	 * if console blocks everything will.  Make sure the error reported
	 * is the one from the syslogd failure.
	 */
	if (_sl_LogStat & LOG_CONS &&
	    (fd = open(_PATH_CONSOLE, O_WRONLY, 0)) >= 0) {
		struct iovec iov[2];
		
		p = strchr(tbuf, '>') + 1;
		iov[0].iov_base = p;
		iov[0].iov_len = cnt - (p - tbuf);
		iov[1].iov_base = "\r\n";
		iov[1].iov_len = 2;
		(void)writev(fd, iov, 2);
		(void)close(fd);
	}
}

#ifndef BUILDING_VARIANT

static struct sockaddr_un SyslogAddr;	/* AF_UNIX address of local logger */

void
openlog(ident, logstat, logfac)
	const char *ident;
	int logstat, logfac;
{
	if (ident != NULL)
		_sl_LogTag = ident;
	_sl_LogStat = logstat;
	if (logfac != 0 && (logfac &~ LOG_FACMASK) == 0)
		_sl_LogFacility = logfac;

	if (_sl_LogFile == -1) {
		SyslogAddr.sun_family = AF_UNIX;
		(void)strncpy(SyslogAddr.sun_path, _PATH_LOG,
		    sizeof(SyslogAddr.sun_path));
		if (_sl_LogStat & LOG_NDELAY) {
			if ((_sl_LogFile = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
				return;
			(void)fcntl(_sl_LogFile, F_SETFD, 1);
		}
	}
	if (_sl_LogFile != -1 && !_sl_connected)
		if (connect(_sl_LogFile, (struct sockaddr *)&SyslogAddr, sizeof(SyslogAddr)) == -1) {
			(void)close(_sl_LogFile);
			_sl_LogFile = -1;
		} else
			_sl_connected = 1;
}

void
closelog()
{
	(void)close(_sl_LogFile);
	_sl_LogFile = -1;
	_sl_connected = 0;
}

/* setlogmask -- set the log mask level */
int
setlogmask(pmask)
	int pmask;
{
	int omask;

	omask = _sl_LogMask;
	if (pmask != 0)
		_sl_LogMask = pmask;
	return (omask);
}

#endif /* !BUILDING_VARIANT */
