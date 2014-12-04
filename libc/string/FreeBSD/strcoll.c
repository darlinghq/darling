/*-
 * Copyright (c) 1995 Alex Tatmanjants <alex@elvisti.kiev.ua>
 *		at Electronni Visti IA, Kiev, Ukraine.
 *			All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/string/strcoll.c,v 1.14 2009/02/03 17:58:20 danger Exp $");

#include "xlocale_private.h"

#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <errno.h>
#include "collate.h"

int
strcoll_l(const char *s, const char *s2, locale_t loc)
{
	int ret;
	const wchar_t *t = NULL, *t2 = NULL;
	int sverrno;

	NORMALIZE_LOCALE(loc);
	if (loc->__collate_load_error || (t = __collate_mbstowcs(s, loc)) == NULL || (t2 = __collate_mbstowcs(s2, loc)) == NULL) {
		sverrno = errno;
		free((void *)t);
		free((void *)t2);
		errno = sverrno;
		return strcmp(s, s2);
	}

	ret = wcscoll_l(t, t2, loc);
	sverrno = errno;
	free((void *)t);
	free((void *)t2);
	errno = sverrno;

	return ret;
}

int
strcoll(const char *s, const char *s2)
{
	return strcoll_l(s, s2, __current_locale());
}
