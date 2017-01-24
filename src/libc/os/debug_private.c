/* Copyright (c) 2012-2013 Apple Inc. All rights reserved.
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

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <mach/mach_time.h>
#include <os/alloc_once_private.h>
#include <os/assumes.h>
#include <os/debug_private.h>
#include <_simple.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/time.h>
#include <unistd.h>

struct os_debug_log_globals_s {
	uint64_t start;
	os_redirect_t redirect;
	int logfd;
	bool prepend_timestamp : 1;
	bool errors_only : 1;
};

// If user picked a filename, use it and only it.
// Otherwise, first try /var/tmp, then $TMPDIR, then give up.
static inline
int
_os_debug_log_open_file(const char *suggestion)
{
	if (suggestion) {
		return open(suggestion, O_WRONLY | O_APPEND | O_CREAT | O_NOFOLLOW |
					O_EXCL | O_CLOEXEC, 0644);
	}

	int fd;
	char filename[PATH_MAX];
	char path[PATH_MAX];

	snprintf(filename, sizeof(filename), "os_debug_log.%s.%d.log", getprogname(),
			 getpid());

	strlcpy(path, "/var/tmp/", sizeof(path));
	if (access(path, W_OK) == 0) {
		strlcat(path, filename, sizeof(path));
		fd = open(path, O_WRONLY | O_APPEND | O_CREAT | O_NOFOLLOW | O_EXCL |
				  O_CLOEXEC, 0644);
		if (fd >= 0) {
			return fd;
		}
	}

	const char *tmpdir = getenv("TMPDIR");
	if (tmpdir) {
		strlcpy(path, tmpdir, sizeof(path));
		if (access(path, W_OK) == 0) {
			strlcat(path, filename, sizeof(path));
			fd = open(path, O_WRONLY | O_APPEND | O_CREAT | O_NOFOLLOW |
					  O_EXCL | O_CLOEXEC, 0644);
			if (fd >= 0) {
				return fd;
			}
		}
	}

	return -1;
}

static
void
_os_debug_log_init(void *globals)
{
	struct os_debug_log_globals_s *g = globals;

	g->errors_only = false;

	g->redirect = dlsym(RTLD_MAIN_ONLY, "_os_debug_log_redirect_func");

	// This is a bit of a hack. LIBDISPATCH_LOG is part of dispatch's API.
	// But now all dispatch logging goes through os_debug_log. So we have to
	// recognize this env var here in Libc.
	// rdar://problem/11685359 tracks deprecating LIBDISPATCH_LOG from dispatch.
	char *e = getenv("LIBDISPATCH_LOG");
	if (!e) {
		e = getenv("OS_DEBUG_LOG");
	}

	// Default log destination
	if (!e || strcmp(e, "YES") == 0) {
#if DEBUG
		e = "file";
#else
		e = "syslog";
#endif
	}

	if (strcmp(e, "NO") == 0) {
		g->logfd = -1;
		g->errors_only = true;
	} else if (strcmp(e, "syslog") == 0) {
		g->logfd = -1;
	} else if (strcmp(e, "stderr") == 0) {
		g->logfd = STDERR_FILENO;
	} else if (strcmp(e, "stdout") == 0) {
		g->logfd = STDOUT_FILENO;
	} else if (strcmp(e, "file") == 0) {
		g->logfd = _os_debug_log_open_file(NULL);
		if (g->logfd == -1) {
			g->errors_only = true;
		}
	} else {
		g->logfd = _os_debug_log_open_file(e);
		if (g->logfd == -1) {
			g->errors_only = true;
		}
	}

	// From now on, g->logfd == -1 means syslog; anything >= 0 is the
	// fd to use. Remember that file descriptor 0 is a perfectly valid
	// value for open() to return if you closed (or never had) stdin.

	// Timestamp every log message if logging directly to file and no
	// redirector is set up.
	if (g->logfd >= 0 && !g->redirect) {
		g->prepend_timestamp = true;

		struct timeval tv;
		gettimeofday(&tv, NULL);

		g->start = mach_absolute_time();

		dprintf(g->logfd,
				"=== os_debug_log log file opened for %s[%u] at %ld.%06u",
				getprogname(), getpid(),
				tv.tv_sec, tv.tv_usec);
		if (g->prepend_timestamp) {
			mach_timebase_info_data_t tbi;
			if (mach_timebase_info(&tbi) == 0) {
				dprintf(g->logfd, " [ns=ticks*%u/%u]",
						tbi.numer, tbi.denom);
			}
		}
		dprintf(g->logfd, " ===\n");
	}
}

#ifndef OS_ALLOC_ONCE_KEY_OS_DEBUG_LOG
#define OS_ALLOC_ONCE_KEY_OS_DEBUG_LOG OS_ALLOC_ONCE_KEY_OS_TRACE
#endif

static inline OS_CONST
struct os_debug_log_globals_s *
os_debug_log_globals(void)
{
	return (struct os_debug_log_globals_s *)
		os_alloc_once(OS_ALLOC_ONCE_KEY_OS_DEBUG_LOG,
					  sizeof(struct os_debug_log_globals_s),
					  _os_debug_log_init);
}

static __attribute__((always_inline))
uint64_t
_os_debug_log_ticks_since_start(void)
{
	return mach_absolute_time() - os_debug_log_globals()->start;
}

// False on error writing to file
static inline
bool
_os_debug_log_write_fd(int level __attribute__((__unused__)),
		    char *str, int fd)
{
	size_t len = strlen(str);

	str[len++] = '\n'; // overwrite null - don't use str*() anymore

	ssize_t rc, wlen = 0;
	do {
		rc = write(fd, &str[wlen], len - wlen);
		if (os_slowpath(rc == -1)) {
			if(errno == EINTR) {
				rc = 0;
			} else {
				return false;
			}
		}
		wlen += rc;
	} while (wlen < len);

	return true;
}

static __attribute__((__noinline__))
void
_os_debug_log_write_error(void)
{
	char err_str[256];
	const char *pfx = "os_debug_log() :";
	size_t pfxlen = strlen(pfx);

	strlcpy(err_str, pfx, sizeof(err_str));
	strerror_r(errno, err_str+pfxlen, sizeof(err_str)-pfxlen);
	_simple_asl_log(LOG_ERR, "com.apple.os_debug_log", err_str);
}

static inline
void
_os_debug_log_write(int level, char *str)
{
	int fd = os_debug_log_globals()->logfd;
	os_redirect_t rdr = os_debug_log_globals()->redirect;
	// true = redirect has fully handled, don't log
	if (os_slowpath(rdr) && os_fastpath(rdr(str))) {
		return;
	}
	if (os_slowpath(fd >= 0)) {
		if (os_fastpath(_os_debug_log_write_fd(level, str, fd))) {
			return;
		} else {
			_os_debug_log_write_error();
			os_debug_log_globals()->logfd = -1;
			// Don't return, fall out to syslog().
		}
	}
	_simple_asl_log(level, "com.apple.os_debug_log", str);
}

static __attribute__((always_inline))
void
_os_debug_logv(int level, const char *msg, va_list ap)
{
	if (os_slowpath((bool)os_debug_log_globals()->errors_only) && level > LOG_ERR) {
		// more important = lower integer
		return;
	}
	char *buf, *freebuf;
	size_t len;

	len = vasprintf(&buf, msg, ap);
	if (!buf) {
		return;
	}
	freebuf = buf;

	// The os_debug_log macros prepend many spaces to the format string.
	// Overwrite them with a timestamp, *or* skip them.
	const size_t pfxlen = strlen(_OS_DEBUG_LOG_PREFIX);
	const size_t timelen = 16;
	__OS_COMPILETIME_ASSERT__(pfxlen >= timelen);

	if (os_fastpath(len > pfxlen)) {
		if (os_slowpath((bool)os_debug_log_globals()->prepend_timestamp)) {
			char tmp = buf[timelen];
			snprintf(buf, timelen + 1, "%16llu", _os_debug_log_ticks_since_start());
			buf[timelen] = tmp; // snprintf's null
		} else {
			buf += pfxlen;
		}
	}

	_os_debug_log_write(level, buf);
	free(freebuf);
}

void
_os_debug_log_error_str(char *msg)
{
	_os_debug_log_write(LOG_ERR, msg);
}

OS_FORMAT_PRINTF(1, 2)
void
_os_debug_log(const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	_os_debug_logv(LOG_DEBUG, msg, ap);
	va_end(ap);
}
