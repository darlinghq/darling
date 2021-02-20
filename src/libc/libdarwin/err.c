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

#pragma mark Utilities
static void
_print_preamble(FILE *fp, const char *fmt, va_list ap)
{
	fprintf(fp, "%s: ", getprogname());
	vfprintf(fp, fmt, ap);
}

#pragma mark API
void
err_np(errno_t code, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verr_np(code, fmt, ap);
	va_end(ap);
}

void
errc_np(int eval, errno_t code, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verrc_np(eval, code, fmt, ap);
	va_end(ap);
}

void
warn_np(errno_t code, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vwarn_np(code, fmt, ap);
	va_end(ap);
}

void
verr_np(errno_t code, const char *fmt, va_list ap)
{
	_print_preamble(stderr, fmt, ap);
	fprintf(stderr, ": %s\n", strerror_np(code));
	exit(sysexit_np(code));
}

void
verrc_np(int eval, errno_t code, const char *fmt, va_list ap)
{
	_print_preamble(stderr, fmt, ap);
	fprintf(stderr, ": %s\n", strerror_np(code));
	exit(eval);
}

void
vwarn_np(errno_t code, const char *fmt, va_list ap)
{
	_print_preamble(stderr, fmt, ap);
	fprintf(stderr, ": %s\n", strerror_np(code));
}
