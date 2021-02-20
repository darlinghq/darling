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

#pragma mark API
int
fcheck_np(FILE *f, size_t n, size_t expected)
{
	if (n == expected) {
		return 0;
	}
	if (feof(f)) {
		return EOF;
	}
	if (ferror(f)) {
		return 1;
	}
	__builtin_unreachable();
}

os_fd_t
dup_np(os_fd_t fd)
{
	os_fd_t dfd = -1;

	while (true) {
		dfd = dup(fd);

		if (os_fd_valid(dfd)) {
			break;
		}

		switch (errno) {
		case EINTR:
			break;
		case EBADF:
			os_crash("bad fd");
		case EMFILE:
		case ENFILE:
			os_crash("failed to dup fd");
		default:
			os_crash("unhandled error: %s", symerror_np(errno));
		}
	}

	return dfd;
}

os_fd_t
claimfd_np(os_fd_t *fdp, const guardid_t *gdid, u_int gdflags)
{
	int ret = -1;
	int fd = *fdp;

	if (gdid) {
		ret = change_fdguard_np(fd, NULL, 0, gdid, gdflags, NULL);
		if (ret) {
			os_crash("change_fdguard_np: %{darwin.errno}d", errno);
		}
	}

	*fdp = -1;
	return fd;
}

os_fd_t
xferfd_np(os_fd_t *fdp, const guardid_t *gdid, u_int gdflags)
{
	int ret = -1;
	int fd = *fdp;

	ret = change_fdguard_np(fd, gdid, gdflags, NULL, 0, NULL);
	if (ret) {
		os_crash("change_fdguard_np: %{darwin.errno}d", errno);
	}

	*fdp = -1;
	return fd;
}

void
close_drop_np(os_fd_t *fdp, const guardid_t *gdid)
{
	int ret = -1;
	int fd = *fdp;

	if (gdid) {
		ret = guarded_close_np(fd, gdid);
	} else {
		ret = close(fd);
	}

	posix_assert_zero(ret);
	*fdp = -1;
}

void
close_drop_optional_np(os_fd_t *fdp, const guardid_t *gdid)
{
	if (!os_fd_valid(*fdp)) {
		return;
	}
	close_drop_np(fdp, gdid);
}

size_t
zsnprintf_np(char *buff, size_t len, const char *fmt, ...)
{
	int np = 0;
	va_list ap;

	va_start(ap, fmt);
	np = vsnprintf(buff, len, fmt, ap);
	va_end(ap);

	if (np < 0) {
		np = 0;
	} else if ((size_t)np >= len) {
		np = (int)len - 1;
	}

	return (size_t)np;
}

void
crfprintf_np(FILE *f, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vcrfprintf_np(f, fmt, ap);
	va_end(ap);
}

void
vcrfprintf_np(FILE *f, const char *fmt, va_list ap)
{
	vfprintf(f, fmt, ap);
	fprintf(f, "\n");
}

void
wfprintf_np(FILE *f, ssize_t initpad, size_t pad, size_t width,
		const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vwfprintf_np(f, initpad, pad, width, fmt, ap);
	va_end(ap);
}

void
vwfprintf_np(FILE *f, ssize_t initpad, size_t pad, size_t width,
		const char *fmt, va_list ap)
{
	char *__os_free string = NULL;
	char *__os_free working = NULL;
	char *__os_free init_padding = NULL;
	char *__os_free padding = NULL;
	const char *curline = NULL;;
	size_t left = 0;
	size_t initpad_labs = (size_t)labs(initpad);
	int ret = -1;

	if (width && width <= pad) {
		os_crash("width cannot be smaller than pad");
	}
	if (width && (initpad > 0) && width <= initpad_labs) {
		os_crash("width cannot be smaller than initpad");
	}
	if (width && (initpad < 0) && width <= initpad_labs) {
		os_crash("width cannot be smaller than negative initpad");
	}

	ret = vasprintf(&string, fmt, ap);
	if (ret < 0 || !string) {
		return;
	}

	left = (size_t)ret;
	curline = string;

	// The working buffer will always be large enough to handle any individual
	// line. vasprintf(3) returns the number of characters printed not including
	// the null terminator, so add space for that.
	working = malloc(left + 1);
	if (!working) {
		return;
	}

	init_padding = malloc(initpad_labs + 1);
	if (!init_padding) {
		return;
	}

	if (initpad >= 0) {
		memset(init_padding, ' ', initpad);
		init_padding[initpad] = 0;
	} else {
		init_padding[0] = 0;
	}

	padding = malloc(pad + 1);
	if (!padding) {
		return;
	}

	memset(padding, ' ', pad);
	padding[pad] = 0;

	do {
		size_t which_pad = pad;
		char *which_padding = padding;
		bool findspace = true;
		size_t n2consume = 0;
		char *breakchar = NULL;

		if (curline == string) {
			which_padding = init_padding;
			which_pad = initpad_labs;
		}

		if (width == 0) {
			// Width is unconstrained so just consume the entire string and
			// indent any new lines within.
			n2consume = left;
			findspace = false;
		} else {
			n2consume = width - which_pad;
			if (n2consume >= left) {
				n2consume = left;
				findspace = false;
			}
		}

		strlcpy(working, curline, n2consume + 1);
		breakchar = strchr(working, '\n');
		if (!breakchar && findspace) {
			// No new line within our maximally-constrained width of characters,
			// so search for a space instead.
			breakchar = strrchr(working, ' ');
		}

		if (breakchar) {
			// Found something to break on, so nerf it and only consume the
			// characters up until that break character.
			*breakchar = 0;
			n2consume = (size_t)(breakchar - working);
			curline += n2consume + 1;
		}

		fprintf(f, "%s%s\n", which_padding, working);
		left -= n2consume;
	} while (left);
}
