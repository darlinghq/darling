/*-
 * Copyright (c) 2001 Alexey Zelkin <phantom@FreeBSD.org>
 * Copyright (c) 1997 FreeBSD Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdtime/timelocal.c,v 1.25 2003/06/13 00:14:07 jkh Exp $");

#include "xlocale_private.h"

#include <stddef.h>
#include <string.h>

#include "ldpart.h"
#include "timelocal.h"

#define LCTIME_SIZE (sizeof(struct lc_time_T) / sizeof(char *))

static const struct lc_time_T	_C_time_locale = {
	{
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	}, {
		"January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December"
	}, {
		"Sun", "Mon", "Tue", "Wed",
		"Thu", "Fri", "Sat"
	}, {
		"Sunday", "Monday", "Tuesday", "Wednesday",
		"Thursday", "Friday", "Saturday"
	},

	/* X_fmt */
	"%H:%M:%S",

	/*
	 * x_fmt
	 * Since the C language standard calls for
	 * "date, using locale's date format," anything goes.
	 * Using just numbers (as here) makes Quakers happier;
	 * it's also compatible with SVR4.
	 */
	"%m/%d/%y",

	/*
	 * c_fmt
	 */
	"%a %b %e %H:%M:%S %Y",

	/* am */
	"AM",

	/* pm */
	"PM",

	/* date_fmt */
	"%a %b %e %H:%M:%S %Z %Y",
	
	/* alt_month
	 * Standalone months forms for %OB
	 */
	{
		"January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December"
	},

	/* md_order
	 * Month / day order in dates
	 */
	"md",

	/* ampm_fmt
	 * To determine 12-hour clock format time (empty, if N/A)
	 */
	"%I:%M:%S %p"
};

__private_extern__ struct lc_time_T *
__get_current_time_locale(locale_t loc)
{
	return (loc->_time_using_locale
		? &loc->__lc_time->_time_locale
		: (struct lc_time_T *)&_C_time_locale);
}

__private_extern__ int
__time_load_locale(const char *name, locale_t loc)
{
	int ret;
	struct __xlocale_st_time *xp;
	static struct __xlocale_st_time *cache = NULL;

	/* 'name' must be already checked. */
	if (strcmp(name, "C") == 0 || strcmp(name, "POSIX") == 0) {
		loc->_time_using_locale = 0;
		XL_RELEASE(loc->__lc_time);
		loc->__lc_time = NULL;
		return (_LDP_CACHE);
	}

	/*
	 * If the locale name is the same as our cache, use the cache.
	 */
	if (cache && cache->_time_locale_buf && strcmp(name, cache->_time_locale_buf) == 0) {
		loc->_time_using_locale = 1;
		XL_RELEASE(loc->__lc_time);
		loc->__lc_time = cache;
		XL_RETAIN(loc->__lc_time);
		return (_LDP_CACHE);
	}
	if ((xp = (struct __xlocale_st_time *)malloc(sizeof(*xp))) == NULL)
		return _LDP_ERROR;
	xp->__refcount = 1;
	xp->__free_extra = (__free_extra_t)__ldpart_free_extra;
	xp->_time_locale_buf = NULL;

	ret = __part_load_locale(name, &loc->_time_using_locale,
			&xp->_time_locale_buf, "LC_TIME",
			LCTIME_SIZE, LCTIME_SIZE,
			(const char **)&xp->_time_locale);
	if (ret == _LDP_LOADED) {
		XL_RELEASE(loc->__lc_time);
		loc->__lc_time = xp;
		XL_RELEASE(cache);
		cache = xp;
		XL_RETAIN(cache);
	} else if (ret == _LDP_ERROR)
		free(xp);

	return (ret);
}
