/*-
 * Copyright (c) 2004 Tim J. Robbins.
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
 *
 * $FreeBSD: src/lib/libc/locale/mblocal.h,v 1.7 2008/01/21 23:48:12 ache Exp $
 */

#ifndef _MBLOCAL_H_
#define	_MBLOCAL_H_

#include <runetype.h>

/*
 * Rune initialization function prototypes.
 */
__private_extern__ int	_none_init(struct __xlocale_st_runelocale *);
__private_extern__ int	_ascii_init(struct __xlocale_st_runelocale *);
__private_extern__ int	_UTF2_init(struct __xlocale_st_runelocale *);
__private_extern__ int	_UTF8_init(struct __xlocale_st_runelocale *);
__private_extern__ int	_EUC_init(struct __xlocale_st_runelocale *);
__private_extern__ int	_GB18030_init(struct __xlocale_st_runelocale *);
__private_extern__ int	_GB2312_init(struct __xlocale_st_runelocale *);
__private_extern__ int	_GBK_init(struct __xlocale_st_runelocale *);
__private_extern__ int	_BIG5_init(struct __xlocale_st_runelocale *);
__private_extern__ int	_MSKanji_init(struct __xlocale_st_runelocale *);

__private_extern__ size_t       _none_mbrtowc(wchar_t * __restrict, const char * __restrict,
                    size_t, mbstate_t * __restrict, locale_t);
__private_extern__ int  _none_mbsinit(const mbstate_t *, locale_t);
__private_extern__ size_t       _none_mbsnrtowcs(wchar_t * __restrict dst,
                    const char ** __restrict src, size_t nms, size_t len,
                    mbstate_t * __restrict ps __unused, locale_t);
__private_extern__ size_t       _none_wcrtomb(char * __restrict, wchar_t,
                    mbstate_t * __restrict, locale_t);
__private_extern__ size_t       _none_wcsnrtombs(char * __restrict, const wchar_t ** __restrict,
                    size_t, size_t, mbstate_t * __restrict, locale_t);

extern size_t __mbsnrtowcs_std(wchar_t * __restrict, const char ** __restrict,
    size_t, size_t, mbstate_t * __restrict, locale_t);
extern size_t __wcsnrtombs_std(char * __restrict, const wchar_t ** __restrict,
    size_t, size_t, mbstate_t * __restrict, locale_t);

#endif	/* _MBLOCAL_H_ */
