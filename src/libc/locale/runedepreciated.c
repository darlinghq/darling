/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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

#include "xlocale_private.h"

#include <string.h>
#include <rune.h>
#include <stdlib.h>
#include <wchar.h>
#include "runedepreciated.h"

__private_extern__ const char __rune_depreciated_msg[] = "\
%s and other functions prototyped in rune.h are depreciated in favor of\n\
the ISO C99 extended multibyte and wide character facilities and should not\n\
be used in new applications.\n\
";

__private_extern__ rune_t
__sgetrune(const char *string, size_t n, char const **result)
{
	wchar_t wc;
	size_t converted = mbrtowc(&wc, string, n, NULL);
	__darwin_rune_t invalid_rune = __current_locale()->__lc_ctype->_CurrentRuneLocale.__invalid_rune;

	switch (converted) {
	case (size_t)-2:	/* incomplete */
		if (result)
			*result = string;
		return invalid_rune;
	case (size_t)-1:	/* invalid */
		if (result)
			*result = string + 1;
		return invalid_rune;
	case (size_t)0:		/* null wide character */
	{
		int i;

		for (i = 1; i < n; i++)
			if (mbrtowc(&wc, string, n, NULL) == (size_t)0)
				break;
		if (result)
			*result = string + i;
		return (rune_t)0;
	}
	default:
		if (result)
			*result = string + converted;
		return (rune_t)wc;
	}
	/* NOTREACHED */
}

__private_extern__ int
__sputrune(rune_t rune, char *string, size_t n, char **result)
{
	char buf[MB_CUR_MAX];
	size_t converted = wcrtomb(buf, rune, NULL);

	if (converted < 0) {
		if (result)
			*result = string;
	} else if (n >= converted) {
		if (string)
			bcopy(buf, string, converted);
		if (result)
			*result = string + converted;
	} else if (result)
		*result = NULL;
	return (converted < 0 ? 0 : converted);
}

__private_extern__ rune_t
sgetrune(const char *string, size_t n, char const **result)
{
	static int warn_depreciated = 1;

	if (warn_depreciated) {
		warn_depreciated = 0;
		fprintf(stderr, __rune_depreciated_msg, "sgetrune");
	}
	return __sgetrune(string, n, result);
}

__private_extern__ int
sputrune(rune_t rune, char *string, size_t n, char **result)
{
	static int warn_depreciated = 1;

	if (warn_depreciated) {
		warn_depreciated = 0;
		fprintf(stderr, __rune_depreciated_msg, "sputrune");
	}
	return __sputrune(rune, string, n, result);
}
