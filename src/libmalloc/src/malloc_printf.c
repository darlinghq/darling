/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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
#include "internal.h"

/* global flag to suppress ASL logging e.g. for syslogd */
int _malloc_no_asl_log = 0;

typedef enum {
	DEBUG_WRITE_NONE,
	DEBUG_WRITE_ON_CRASH,
	DEBUG_WRITE_ALWAYS,
} write_debug_mode_t;

static const char Malloc_Facility[] = "com.apple.Libsystem.malloc";
static int malloc_debug_file = STDERR_FILENO;
static write_debug_mode_t debug_mode = DEBUG_WRITE_NONE;
static boolean_t malloc_error_stop;		// Stop when reporting error.
static boolean_t malloc_error_sleep;	// Sleep after reporting error.
static const int default_sleep_time = 3600;

// Gets the default time to sleep for when reporting an error. Returns 0
// (meaning do not sleep) if malloc_error_sleep is 0 (that is, if sleeping on
// error is not configured).
MALLOC_INLINE MALLOC_ALWAYS_INLINE
static unsigned _malloc_default_debug_sleep_time()
{
	return malloc_error_sleep ? default_sleep_time : 0;
}

#define WRITE_TO_DEBUG_FILE(flags) \
		((debug_mode == DEBUG_WRITE_ALWAYS) || \
		(debug_mode == DEBUG_WRITE_ON_CRASH && (flags & MALLOC_REPORT_CRASH)))
#define	MALLOC_REPORT_LEVEL_MASK	0x0f

#pragma mark -
#pragma mark Configuration

void
malloc_print_configure(bool restricted)
{
	char *flag = getenv("MallocDebugReport");
	if (flag) {
		if (!strcmp(flag, "stderr")) {
			debug_mode = DEBUG_WRITE_ALWAYS;
		} else if (!strcmp(flag, "crash")) {
			debug_mode = DEBUG_WRITE_ON_CRASH;
		} else if (!strcmp(flag, "none")) {
			debug_mode = DEBUG_WRITE_NONE;
		} else {
			debug_mode = DEBUG_WRITE_ALWAYS;
			malloc_printf("Unrecognized value for MallocDebugReport (%s) - using 'stderr'\n", flag);
		}
	} else {
		// Default is to write to stderr only if it's a tty.
		if (isatty(STDERR_FILENO)) {
			debug_mode = DEBUG_WRITE_ALWAYS;
		}
	}
	if (getenv("MallocErrorStop")) {
		malloc_error_stop = TRUE;
	}
	if (getenv("MallocErrorSleep")) {
		malloc_error_sleep = TRUE;
	}
}

#pragma mark -
#pragma mark Low level debug output

/*
 * The functions that follow use _simple_*printf.  They deal with a
 * subset of printf format specifiers and do not call malloc internally.
 */
static void
_malloc_put(uint32_t flags, const char *msg)
{
	_SIMPLE_STRING b;
	if ((b = _simple_salloc()) == NULL) {
		if (WRITE_TO_DEBUG_FILE(flags)) {
			if (!(flags & MALLOC_REPORT_NOPREFIX)) {
				void *self = _os_tsd_get_direct(__TSD_THREAD_SELF);
				_simple_dprintf(malloc_debug_file, "%s(%d,%p) malloc: ", getprogname(), getpid(), self);
			}
			write(malloc_debug_file, msg, strlen(msg));
		}
		return;
	}
	if (!(flags & MALLOC_REPORT_NOPREFIX)) {
		void *self = _os_tsd_get_direct(__TSD_THREAD_SELF);
		_simple_sprintf(b, "%s(%d,%p) malloc: ", getprogname(), getpid(), self);
	}

	_simple_sprintf(b, "%s", msg);
	if (WRITE_TO_DEBUG_FILE(flags)) {
		_simple_put(b, malloc_debug_file);
	}
	if (_malloc_no_asl_log & !(flags & MALLOC_REPORT_NOLOG)) {
		_simple_asl_log(flags & MALLOC_REPORT_LEVEL_MASK, Malloc_Facility, _simple_string(b));
	}
	_simple_sfree(b);
}

#pragma mark -
#pragma mark High-Level Reporting Functions

MALLOC_NOINLINE void
malloc_vreport(uint32_t flags, unsigned sleep_time, const char *prefix_msg,
		const void *prefix_arg, const char *fmt, va_list ap)
{
	const char *crash_msg = NULL;
	_SIMPLE_STRING b = NULL;
	if ((b = _simple_salloc()) == NULL) {
		if (WRITE_TO_DEBUG_FILE(flags)) {
			if (!(flags & MALLOC_REPORT_NOPREFIX)) {
				void *self = _os_tsd_get_direct(__TSD_THREAD_SELF);
				_simple_dprintf(malloc_debug_file, "%s(%d,%p) malloc: ", getprogname(), getpid(), self);
			}
			if (prefix_msg) {
				_simple_dprintf(malloc_debug_file, prefix_msg, prefix_arg);
			}
			_simple_vdprintf(malloc_debug_file, fmt, ap);
		}
		if (flags & MALLOC_REPORT_CRASH) {
			crash_msg = fmt;
		}
	} else {
		if (!(flags & MALLOC_REPORT_NOPREFIX)) {
			void *self = _os_tsd_get_direct(__TSD_THREAD_SELF);
			_simple_sprintf(b, "%s(%d,%p) malloc: ", getprogname(), getpid(), self);
		}
		if (prefix_msg) {
			_simple_sprintf(b, prefix_msg, prefix_arg);
		}
		_simple_vsprintf(b, fmt, ap);
		if (WRITE_TO_DEBUG_FILE(flags)) {
			_simple_put(b, malloc_debug_file);
		}
		if (!_malloc_no_asl_log && !(flags & MALLOC_REPORT_NOLOG)) {
			_simple_asl_log(flags & MALLOC_REPORT_LEVEL_MASK, Malloc_Facility, _simple_string(b));
		}
		if (flags & MALLOC_REPORT_CRASH) {
			crash_msg = _simple_string(b);
		} else {
			_simple_sfree(b);
		}
	}

	if (flags & (MALLOC_REPORT_DEBUG | MALLOC_REPORT_CRASH)) {
		_malloc_put(flags, "*** set a breakpoint in malloc_error_break to debug\n");
		malloc_error_break();

		if (malloc_error_stop) {
			_malloc_put(ASL_LEVEL_NOTICE, "*** sending SIGSTOP to help debug\n");
			kill(getpid(), SIGSTOP);
		} else if (sleep_time) {
			_malloc_put(ASL_LEVEL_NOTICE, "*** sleeping to help debug\n");
			sleep(sleep_time);
		}
	}

	if (flags & MALLOC_REPORT_CRASH) {
		_os_set_crash_log_message_dynamic(crash_msg);
		abort();
	}
}

MALLOC_NOEXPORT void
malloc_report(uint32_t flags, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	malloc_vreport(flags, _malloc_default_debug_sleep_time(), NULL, NULL, fmt, ap);
	va_end(ap);
}

MALLOC_NOEXPORT void
malloc_report_simple(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	malloc_vreport(MALLOC_REPORT_NOLOG | MALLOC_REPORT_NOPREFIX,
			_malloc_default_debug_sleep_time(), NULL, NULL, fmt, ap);
	va_end(ap);
}

#pragma mark -
#pragma mark Zone Error Reporing

void
malloc_zone_error(uint32_t flags, bool is_corruption, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	uint32_t report_flags = MALLOC_REPORT_DEBUG | MALLOC_REPORT_NOLOG;
	if ((is_corruption && (flags & MALLOC_ABORT_ON_CORRUPTION)) ||
			(flags & MALLOC_ABORT_ON_ERROR)) {
		report_flags = MALLOC_REPORT_CRASH;
	}
	malloc_vreport(report_flags | ASL_LEVEL_ERR, _malloc_default_debug_sleep_time(),
			NULL, NULL, fmt, ap);
	va_end(ap);
}

#pragma mark -
#pragma mark Malloc Output API.

// malloc_printf() needs to be retained and exported because it's API (defined
// in malloc/malloc.h). It's equivalent to calling malloc_report() with
// a flags value of ASL_LEVEL_ERR, so does not result in a crash or any prompts
// for diagnostics or breakpoints.
// Do not use in malloc code.
void
malloc_printf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	malloc_vreport(ASL_LEVEL_ERR, 0, NULL, NULL, fmt, ap);
	va_end(ap);
}
