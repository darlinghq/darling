/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#define	MALLOC_REPORT_NOLOG			0x10
#define	MALLOC_REPORT_NOPREFIX		0x20
#define MALLOC_REPORT_CRASH			0x40
#define MALLOC_REPORT_DEBUG			0x80

// Most internal logging should use malloc_report() or malloc_vreport(). The
// flags argument should be a combination of the MALLOC_REPORT_xxx values and
// an optional log level encoded using the ASL_LEVEL_xxx constants. The log
// level is ignored if MALLOC_REPORT_NOLOG is set.
//
// The flags do the following:
//	MALLOC_REPORT_NOLOG:
//			Does not send the text to _simple_asl_log().
//	MALLOC_REPORT_NO_PREFIX:
//			Does not write the program name, pid and thread identifier before
//			the report text.
//	MALLOC_REPORTDEBUG:
//			includes text suggesting that a breakpoint could be set
//			on malloc_error_break() to debug this kind of error.
//	MALLOC_REPORT_CRASH:
//			Same as MALLOC_REPORTDEBUG, but crashes after writing the report
//			message.
//
// In addition, if MALLOC_REPORT_CRASH or MALLOC_REPORTDEBUG are specified, this
// function will sleep for an hour or send a SIGSTOP signal to the process if
// the MallocErrorSleep and MallocErrorStop environment variables were set and
// the report text will include a message indicating that this is
// happening. In the case of MALLOC_REPORT_CRASH, the crash occurs after all of
// the other actions have completed.
MALLOC_NOEXPORT MALLOC_NOINLINE void
malloc_report(uint32_t flags, const char *fmt, ...) __printflike(2,3);

// Like malloc_report(), but does not send the text to _simple_asl_log() and
// does not write the program name, pid and thread identifier before the report
// text. Equivalent to malloc_report(MALLOC_REPORT_NOLOG|MALLOC_REPORT_NO_PREFIX)
MALLOC_NOEXPORT MALLOC_NOINLINE void
malloc_report_simple(const char *fmt, ...) __printflike(1,2);

// Like malloc_report(), but precedes the output message with prefix_msg
// as a format string using prefix_arg as a single substition parameter,
// allows the length of time to sleep while reporting an error to be
// specified and passes the arguments to the fmt parameter in a va_list.
MALLOC_NOEXPORT MALLOC_NOINLINE void
malloc_vreport(uint32_t flags, unsigned sleep_time, const char *prefix_msg,
		const void *prefix_arg, const char *fmt, va_list ap);

// Higher-level functions used by zone implementations to report errors.
MALLOC_NOEXPORT MALLOC_NOINLINE void
malloc_zone_error(uint32_t flags, bool is_corruption, const char *fmt, ...) __printflike(3,4);

MALLOC_NOEXPORT MALLOC_NOINLINE void
malloc_zone_check_fail(const char *msg, const char *fmt, ...) __printflike(2,3);

// Configures where malloc logging goes based on environment variables. By
// default, goes to stderr if it's a tty, and is otherwise dropped.
MALLOC_NOEXPORT void
malloc_print_configure(bool restricted);
