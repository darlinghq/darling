/*
 * Copyright (c) 2005, 2008 Apple Inc. All rights reserved.
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

#ifndef _XLOCALE_PRIVATE_H_
#define _XLOCALE_PRIVATE_H_

#include <sys/cdefs.h>
#define __DARWIN_XLOCALE_PRIVATE
#include <xlocale.h>
#undef __DARWIN_XLOCALE_PRIVATE
#include <stdlib.h>
#include <locale.h>
#include <libkern/OSAtomic.h>
#include <pthread.h>
#include <pthread_spinlock.h>
#include <limits.h>
#include "setlocale.h"
#include "collate.h"
#include "runetype.h"
#include "lmessages.h"
#include "lmonetary.h"
#include "lnumeric.h"
#include "timelocal.h"

#undef MB_CUR_MAX
#define MB_CUR_MAX	(__current_locale()->__lc_ctype->__mb_cur_max)
#undef MB_CUR_MAX_L
#define MB_CUR_MAX_L(x)	((x)->__lc_ctype->__mb_cur_max)

extern int __is_threaded;

typedef void (*__free_extra_t)(void *);

#define XPERMANENT	((__free_extra_t)-1)
#define XMAGIC		0x786c6f63616c6530LL	/* 'xlocale0' */

#define	__STRUCT_COMMON	\
	int32_t __refcount; \
	__free_extra_t __free_extra;

struct __xlocale_st_collate {
	__STRUCT_COMMON
	char __encoding[ENCODING_LEN + 1];
	struct __collate_st_info __info;
	struct __collate_st_subst *__substitute_table[COLL_WEIGHTS_MAX];
	struct __collate_st_chain_pri *__chain_pri_table;
	struct __collate_st_large_char_pri *__large_char_pri_table;
	struct __collate_st_char_pri __char_pri_table[UCHAR_MAX + 1];
};
struct __xlocale_st_runelocale {
	__STRUCT_COMMON
	char __ctype_encoding[ENCODING_LEN + 1];
	int __mb_cur_max;
	int __mb_sb_limit;
	size_t (*__mbrtowc)(wchar_t * __restrict, const char * __restrict,
	    size_t, __darwin_mbstate_t * __restrict, struct _xlocale *);
	int (*__mbsinit)(const __darwin_mbstate_t *, struct _xlocale *);
	size_t (*__mbsnrtowcs)(wchar_t * __restrict, const char ** __restrict,
	    size_t, size_t, __darwin_mbstate_t * __restrict, struct _xlocale *);
	size_t (*__wcrtomb)(char * __restrict, wchar_t,
	    __darwin_mbstate_t * __restrict, struct _xlocale *);
	size_t (*__wcsnrtombs)(char * __restrict, const wchar_t ** __restrict,
	    size_t, size_t, __darwin_mbstate_t * __restrict, struct _xlocale *);
	int __datasize;
	_RuneLocale _CurrentRuneLocale;
};
struct __xlocale_st_ldpart {
	__STRUCT_COMMON
	char *_locale_buf;
};
/*
 * the next four structures must have the first three fields of the same
 * as the _xlocale_st_ldpart structure above.
 */
struct __xlocale_st_messages {
	__STRUCT_COMMON
	char *_messages_locale_buf;
	struct lc_messages_T _messages_locale;
};
struct __xlocale_st_monetary {
	__STRUCT_COMMON
	char *_monetary_locale_buf;
	struct lc_monetary_T _monetary_locale;
};
struct __xlocale_st_numeric {
	__STRUCT_COMMON
	char *_numeric_locale_buf;
	struct lc_numeric_T _numeric_locale;
};
struct __xlocale_st_time {
	__STRUCT_COMMON
	char *_time_locale_buf;
	struct lc_time_T _time_locale;
};

/* the extended locale structure */
    /* values for __numeric_fp_cvt */
#define	LC_NUMERIC_FP_UNINITIALIZED	0
#define	LC_NUMERIC_FP_SAME_LOCALE	1
#define	LC_NUMERIC_FP_USE_LOCALE	2

struct _xlocale {
/* The item(s) before __magic are not copied when duplicating locale_t's */
	__STRUCT_COMMON	/* only used for locale_t's in __lc_numeric_loc */
	/* 10 independent mbstate_t buffers! */
	__darwin_mbstate_t __mbs_mblen;
	__darwin_mbstate_t __mbs_mbrlen;
	__darwin_mbstate_t __mbs_mbrtowc;
	__darwin_mbstate_t __mbs_mbsnrtowcs;
	__darwin_mbstate_t __mbs_mbsrtowcs;
	__darwin_mbstate_t __mbs_mbtowc;
	__darwin_mbstate_t __mbs_wcrtomb;
	__darwin_mbstate_t __mbs_wcsnrtombs;
	__darwin_mbstate_t __mbs_wcsrtombs;
	__darwin_mbstate_t __mbs_wctomb;
	pthread_lock_t __lock;
/* magic (Here up to the end is copied when duplicating locale_t's) */
	int64_t __magic;
/* flags */
	unsigned char __collate_load_error;
	unsigned char __collate_substitute_nontrivial;
	unsigned char _messages_using_locale;
	unsigned char _monetary_using_locale;
	unsigned char _numeric_using_locale;
	unsigned char _time_using_locale;
	unsigned char __mlocale_changed;
	unsigned char __nlocale_changed;
	unsigned char __numeric_fp_cvt;
/* collate */
	struct __xlocale_st_collate *__lc_collate;
/* ctype */
	struct __xlocale_st_runelocale *__lc_ctype;
/* messages */
	struct __xlocale_st_messages *__lc_messages;
/* monetary */
	struct __xlocale_st_monetary *__lc_monetary;
/* numeric */
	struct __xlocale_st_numeric *__lc_numeric;
	struct _xlocale *__lc_numeric_loc;
/* time */
	struct __xlocale_st_time *__lc_time;
/* localeconv */
	struct lconv __lc_localeconv;
};

#define NORMALIZE_LOCALE(x)	if ((x) == NULL) { \
					(x) = _c_locale; \
				} else if ((x) == LC_GLOBAL_LOCALE) { \
					(x) = &__global_locale; \
				}

#define XL_LOCK(x)	LOCK((x)->__lock);
#define	XL_RELEASE(x)	if ((x) && (x)->__free_extra != XPERMANENT && OSAtomicDecrement32Barrier(&(x)->__refcount) == 0) { \
				if ((x)->__free_extra) \
					(*(x)->__free_extra)((x)); \
				free((x)); \
			}
#define	XL_RETAIN(x)	if ((x) && (x)->__free_extra != XPERMANENT) { OSAtomicIncrement32Barrier(&(x)->__refcount); }
#define XL_UNLOCK(x)	UNLOCK((x)->__lock);

__private_extern__ struct __xlocale_st_runelocale _DefaultRuneXLocale;
__private_extern__ struct _xlocale	__global_locale;
__private_extern__ pthread_key_t	__locale_key;

__BEGIN_DECLS

void	__ldpart_free_extra(struct __xlocale_st_ldpart *);
locale_t __numeric_ctype(locale_t);
void	__xlocale_init(void);

static inline __attribute__((always_inline)) locale_t
__current_locale(void)
{
	locale_t __locale = (locale_t)pthread_getspecific(__locale_key);
	return (__locale ? __locale : &__global_locale);
}

static inline __attribute__((always_inline)) locale_t
__locale_ptr(locale_t __loc)
{
	NORMALIZE_LOCALE(__loc);
	return __loc;
}

__END_DECLS

#endif /* _XLOCALE_PRIVATE_H_ */
