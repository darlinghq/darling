/*-
 * Copyright (c) 2002 Tim J. Robbins
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
__FBSDID("$FreeBSD: src/lib/libc/string/wcscoll.c,v 1.3 2004/04/07 09:47:56 tjr Exp $");

#include "xlocale_private.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "collate.h"

#define NOTFORWARD	(DIRECTIVE_BACKWARD | DIRECTIVE_POSITION)

int
wcscoll_l(const wchar_t *ws1, const wchar_t *ws2, locale_t loc)
{
	int sverrno;
	int len, len2, prim, prim2, sec, sec2, ret, ret2;
	const wchar_t *t, *t2;
	wchar_t *tt = NULL, *tt2 = NULL;
	wchar_t *tr = NULL, *tr2 = NULL;
	wchar_t w, w2;
	struct __collate_st_info *info;

	NORMALIZE_LOCALE(loc);
	if (loc->__collate_load_error)
		/*
		 * Locale has no special collating order or could not be
		 * loaded, do a fast binary comparison.
		 */
		return (wcscmp(ws1, ws2));

	info = &loc->__lc_collate->__info;
	len = len2 = 1;
	ret = ret2 = 0;

	if ((info->directive[0] & NOTFORWARD) ||
	    (info->directive[1] & NOTFORWARD) ||
	    (!(info->flags && COLLATE_SUBST_DUP) &&
	    (info->subst_count[0] > 0 || info->subst_count[1] > 0))) {
		int direc, pass;
		for(pass = 0; pass < info->directive_count; pass++) {
			direc = info->directive[pass];
			if (pass == 0 || !(info->flags & COLLATE_SUBST_DUP)) {
				free(tt);
				tt = __collate_substitute(ws1, pass, loc);
				free(tt2);
				tt2 = tt ? __collate_substitute(ws2, pass, loc) : NULL;
			}
			if (direc & DIRECTIVE_BACKWARD) {
				wchar_t *bp, *fp, c;
				tr = __collate_wcsdup(tt ? tt : ws1);
				bp = tr;
				fp = tr + wcslen(tr) - 1;
				while(bp < fp) {
					c = *bp;
					*bp++ = *fp;
					*fp-- = c;
				}
				tr2 = __collate_wcsdup(tt2 ? tt2 : ws2);
				bp = tr2;
				fp = tr2 + wcslen(tr2) - 1;
				while(bp < fp) {
					c = *bp;
					*bp++ = *fp;
					*fp-- = c;
				}
				t = (const wchar_t *)tr;
				t2 = (const wchar_t *)tr2;
			} else if (tt) {
				t = (const wchar_t *)tt;
				t2 = (const wchar_t *)tt2;
			} else {
				t = (const wchar_t *)ws1;
				t2 = (const wchar_t *)ws2;
			}
			if(direc & DIRECTIVE_POSITION) {
				while(*t && *t2) {
					prim = prim2 = 0;
					__collate_lookup_which(t, &len, &prim, pass, loc);
					if (prim <= 0) {
						if (prim < 0) {
							errno = EINVAL;
							ret = -1;
							goto end;
						}
						prim = COLLATE_MAX_PRIORITY;
					}
					__collate_lookup_which(t2, &len2, &prim2, pass, loc);
					if (prim2 <= 0) {
						if (prim2 < 0) {
							errno = EINVAL;
							ret = -1;
							goto end;
						}
						prim2 = COLLATE_MAX_PRIORITY;
					}
					if(prim != prim2) {
						ret = prim - prim2;
						goto end;
					}
					t += len;
					t2 += len2;
				}
			} else {
				while(*t && *t2) {
					prim = prim2 = 0;
					while(*t) {
						__collate_lookup_which(t, &len, &prim, pass, loc);
						if(prim > 0)
							break;
						if (prim < 0) {
							errno = EINVAL;
							ret = -1;
							goto end;
						}
						t += len;
					}
					while(*t2) {
						__collate_lookup_which(t2, &len2, &prim2, pass, loc);
						if(prim2 > 0)
							break;
						if (prim2 < 0) {
							errno = EINVAL;
							ret = -1;
							goto end;
						}
						t2 += len2;
					}
					if(!prim || !prim2)
						break;
					if(prim != prim2) {
						ret = prim - prim2;
						goto end;
					}
					t += len;
					t2 += len2;
				}
			}
			if(!*t) {
				if(*t2) {
					ret = -(int)*t2;
					goto end;
				}
			} else {
				ret = *t;
				goto end;
			}
		}
		ret = 0;
		goto end;
	}

	/* optimized common case: order_start forward;forward and duplicate
	 * (or no) substitute tables */
	tt = __collate_substitute(ws1, 0, loc);
	if (tt == NULL) {
		tt2 = NULL;
		t = (const wchar_t *)ws1;
		t2 = (const wchar_t *)ws2;
	} else {
		tt2 = __collate_substitute(ws2, 0, loc);
		t = (const wchar_t *)tt;
		t2 = (const wchar_t *)tt2;
	}
	while(*t && *t2) {
		prim = prim2 = 0;
		while(*t) {
			__collate_lookup_l(t, &len, &prim, &sec, loc);
			if (prim > 0)
				break;
			if (prim < 0) {
				errno = EINVAL;
				ret = -1;
				goto end;
			}
			t += len;
		}
		while(*t2) {
			__collate_lookup_l(t2, &len2, &prim2, &sec2, loc);
			if (prim2 > 0)
				break;
			if (prim2 < 0) {
				errno = EINVAL;
				ret = -1;
				goto end;
			}
			t2 += len2;
		}
		if(!prim || !prim2)
			break;
		if(prim != prim2) {
			ret = prim - prim2;
			goto end;
		}
		if(!ret2)
			ret2 = sec - sec2;
		t += len;
		t2 += len2;
	}
	if(!*t && *t2)
		ret = -(int)*t2;
	else if(*t && !*t2)
		ret = *t;
	else if(!*t && !*t2)
		ret = ret2;
  end:
	sverrno = errno;
	free(tt);
	free(tt2);
	free(tr);
	free(tr2);
	errno = sverrno;

	return ret;
}

int
wcscoll(const wchar_t *ws1, const wchar_t *ws2)
{
	return wcscoll_l(ws1, ws2, __current_locale());
}
