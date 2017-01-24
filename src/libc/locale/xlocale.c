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

#include "xlocale_private.h"
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include "ldpart.h"

#define NMBSTATET	10
#define C_LOCALE_INITIALIZER	{	\
	0, XPERMANENT,			\
	{}, {}, {}, {}, {},		\
	{}, {}, {}, {}, {},		\
	LOCK_INITIALIZER,		\
	XMAGIC,				\
	1, 0, 0, 0, 0, 0, 1, 1, 0,	\
	NULL,				\
	&_DefaultRuneXLocale,		\
}

static char C[] = "C";
static struct _xlocale __c_locale = C_LOCALE_INITIALIZER;
const locale_t _c_locale = (const locale_t)&__c_locale;
struct _xlocale __global_locale = C_LOCALE_INITIALIZER;
pthread_key_t __locale_key = (pthread_key_t)-1;

extern int __collate_load_tables(const char *, locale_t);
extern int __detect_path_locale(void);
extern const char *__get_locale_env(int);
extern int __messages_load_locale(const char *, locale_t);
extern int __monetary_load_locale(const char *, locale_t);
extern int __numeric_load_locale(const char *, locale_t);
extern int __setrunelocale(const char *, locale_t);
extern int __time_load_locale(const char *, locale_t);

static void _releaselocale(locale_t loc);

/*
 * check that the encoding is the right size, isn't . or .. and doesn't
 * contain any slashes
 */
static inline __attribute__((always_inline)) int
_checkencoding(const char *encoding)
{
	return (encoding && (strlen(encoding) > ENCODING_LEN
	  || (encoding[0] == '.' && (encoding[1] == 0
	  || (encoding[1] == '.' && encoding[2] == 0)))
	  || strchr(encoding, '/') != NULL)) ? -1 : 0;
}

/*
 * check that the locale has the right magic number
 */
static inline __attribute__((always_inline)) int
_checklocale(const locale_t loc)
{
	if (!loc)
		return 0;
	return (loc == LC_GLOBAL_LOCALE || loc->__magic == XMAGIC) ? 0 : -1;
}

/*
 * copy a locale_t except anything before the magic value
 */
static inline __attribute__((always_inline)) void
_copylocale(locale_t dst, const locale_t src)
{
	memcpy(&dst->__magic, &src->__magic, sizeof(*dst) - offsetof(struct _xlocale, __magic));
}

/*
 * Make a copy of a locale_t, locking/unlocking the source.
 * A NULL locale_t means to make a copy of the current
 * locale while LC_GLOBAL_LOCALE means to copy the global locale.  If
 * &__c_locale is passed (meaning a C locale is desired), just make
 * a copy.
 */
static locale_t
_duplocale(locale_t loc)
{
	locale_t new;

	if ((new = (locale_t)malloc(sizeof(struct _xlocale))) == NULL)
		return NULL;
	new->__refcount = 1;
	new->__free_extra = (__free_extra_t)_releaselocale;
	new->__lock = LOCK_INITIALIZER;
	if (loc == NULL)
		loc = __current_locale();
	else if (loc == LC_GLOBAL_LOCALE)
		loc = &__global_locale;
	else if (loc == &__c_locale) {
		*new = __c_locale;
		new->__refcount = 1;
		new->__free_extra = (__free_extra_t)_releaselocale;
		new->__lock = LOCK_INITIALIZER;
		return new;
	}
	XL_LOCK(loc);
	_copylocale(new, loc);
	XL_UNLOCK(loc);
	/* __mbs_mblen is the first of NMBSTATET mbstate_t buffers */
	bzero(&new->__mbs_mblen, offsetof(struct _xlocale, __magic)
	    - offsetof(struct _xlocale, __mbs_mblen));
	/* collate */
	XL_RETAIN(new->__lc_collate);
	/* ctype */
	XL_RETAIN(new->__lc_ctype);
	/* messages */
	XL_RETAIN(new->__lc_messages);
	/* monetary */
	XL_RETAIN(new->__lc_monetary);
	/* numeric */
	XL_RETAIN(new->__lc_numeric);
	XL_RETAIN(new->__lc_numeric_loc);
	/* time */
	XL_RETAIN(new->__lc_time);

	return new;
}

/*
 * Modify a locale_t, setting the parts specified in the mask
 * to the locale specified by the string.  If the string is NULL, the C
 * locale is used.  If the string is empty, the value is determined from
 * the environment.  -1 is returned on error, and loc is in a partial state.
 */
static int
_modifylocale(locale_t loc, int mask, __const char *locale)
{
	int m, ret;
	const char *enc = NULL;
	char *oenc;

	if (!locale)
		locale = C;

	ret = __detect_path_locale();
	if (ret) {
		errno = ret;
		return -1;
	}

	if (*locale)
		enc = locale;
	for(m = 1; m <= _LC_LAST_MASK; m <<= 1) {
		if (m & mask) {
			switch(m) {
			case LC_COLLATE_MASK:
				if (!*locale) {
					enc = __get_locale_env(LC_COLLATE);
					if (_checkencoding(enc) < 0) {
						errno = EINVAL;
						return -1;
					}
				}
				oenc = (loc->__collate_load_error ? C : loc->__lc_collate->__encoding);
				if (strcmp(enc, oenc) != 0 && __collate_load_tables(enc, loc) == _LDP_ERROR)
					return -1;
				break;
			case LC_CTYPE_MASK:
				if (!*locale) {
					enc = __get_locale_env(LC_CTYPE);
					if (_checkencoding(enc) < 0) {
						errno = EINVAL;
						return -1;
					}
				}
				if (strcmp(enc, loc->__lc_ctype->__ctype_encoding) != 0) {
					if ((ret = __setrunelocale(enc, loc)) != 0) {
						errno = ret;
						return -1;
					}
					if (loc->__numeric_fp_cvt == LC_NUMERIC_FP_SAME_LOCALE)
						loc->__numeric_fp_cvt = LC_NUMERIC_FP_UNINITIALIZED;
				}
				break;
			case LC_MESSAGES_MASK:
				if (!*locale) {
					enc = __get_locale_env(LC_MESSAGES);
					if (_checkencoding(enc) < 0) {
						errno = EINVAL;
						return -1;
					}
				}
				oenc = (loc->_messages_using_locale ? loc->__lc_messages->_messages_locale_buf : C);
				if (strcmp(enc, oenc) != 0 && __messages_load_locale(enc, loc) == _LDP_ERROR)
					return -1;
				break;
			case LC_MONETARY_MASK:
				if (!*locale) {
					enc = __get_locale_env(LC_MONETARY);
					if (_checkencoding(enc) < 0) {
						errno = EINVAL;
						return -1;
					}
				}
				oenc = (loc->_monetary_using_locale ? loc->__lc_monetary->_monetary_locale_buf : C);
				if (strcmp(enc, oenc) != 0 && __monetary_load_locale(enc, loc) == _LDP_ERROR)
					return -1;
				break;
			case LC_NUMERIC_MASK:
				if (!*locale) {
					enc = __get_locale_env(LC_NUMERIC);
					if (_checkencoding(enc) < 0) {
						errno = EINVAL;
						return -1;
					}
				}
				oenc = (loc->_numeric_using_locale ? loc->__lc_numeric->_numeric_locale_buf : C);
				if (strcmp(enc, oenc) != 0) {
					if (__numeric_load_locale(enc, loc) == _LDP_ERROR)
						return -1;
					loc->__numeric_fp_cvt = LC_NUMERIC_FP_UNINITIALIZED;
					XL_RELEASE(loc->__lc_numeric_loc);
					loc->__lc_numeric_loc = NULL;
				}
				break;
			case LC_TIME_MASK:
				if (!*locale) {
					enc = __get_locale_env(LC_TIME);
					if (_checkencoding(enc) < 0) {
						errno = EINVAL;
						return -1;
					}
				}
				oenc = (loc->_time_using_locale ? loc->__lc_time->_time_locale_buf : C);
				if (strcmp(enc, oenc) != 0 && __time_load_locale(enc, loc) == _LDP_ERROR)
					return -1;
				break;
			}
		}
	}
	return 0;
}

/*
 * release all the memory objects (the memory will be freed when the refcount
 * becomes zero)
 */
static void
_releaselocale(locale_t loc)
{
	/* collate */
	XL_RELEASE(loc->__lc_collate);
	/* ctype */
	XL_RELEASE(loc->__lc_ctype);
	/* messages */
	XL_RELEASE(loc->__lc_messages);
	/* monetary */
	XL_RELEASE(loc->__lc_monetary);
	/* numeric */
	XL_RELEASE(loc->__lc_numeric);
	XL_RELEASE(loc->__lc_numeric_loc);
	/* time */
	XL_RELEASE(loc->__lc_time);
}

/*
 * EXTERNAL: Duplicate a (non-NULL) locale_t.  LC_GLOBAL_LOCALE means the
 * global locale, while NULL means the current locale.  NULL is returned
 * on error.
 */
locale_t
duplocale(locale_t loc)
{
	if (_checklocale(loc) < 0) {
		errno = EINVAL;
		return NULL;
	}
	return _duplocale(loc);
}

/*
 * EXTERNAL: Free a locale_t, releasing all memory objects.  Don't free
 * illegal locale_t's or the global locale.
 */
int
freelocale(locale_t loc)
{
	if (!loc || _checklocale(loc) < 0 || loc == &__global_locale
	    || loc == LC_GLOBAL_LOCALE || loc == &__c_locale) {
		errno = EINVAL;
		return -1;
	}
	XL_RELEASE(loc);
	return 0;
}

/*
 * EXTERNAL: Create a new locale_t, based on the base locale_t, and modified
 * by the mask and locale string.  If the base is NULL, the current locale
 * is used as the base.  If locale is NULL, changes are made from the C locale
 * for categories set in mask.
 */
locale_t
newlocale(int mask, __const char *locale, locale_t base)
{
	locale_t new;
	int lcmask = (mask & LC_ALL_MASK);

	if (_checkencoding(locale) < 0) {
		errno = EINVAL;
		return NULL;
	}
	if (lcmask == LC_ALL_MASK)
		base = (locale_t)&__c_locale;
	else if (_checklocale(base) < 0) {
		errno = EINVAL;
		return NULL;
	}
	new = _duplocale(base);
	if (new == NULL)
		return NULL;
	if (lcmask == 0 || (lcmask == LC_ALL_MASK && locale == NULL))
		return new;
	if (_modifylocale(new, lcmask, locale) < 0) {
		freelocale(new);
		return NULL;
	}
	return new;
}

/*
 * PRIVATE EXTERNAL: Returns the locale that can be used by wcstod and
 * family, to convert the wide character string to a multi-byte string
 * (the LC_NUMERIC and LC_CTYPE locales may be different).
 */
__private_extern__ locale_t
__numeric_ctype(locale_t loc)
{
	switch(loc->__numeric_fp_cvt) {
	case LC_NUMERIC_FP_UNINITIALIZED: {
		const char *ctype = loc->__lc_ctype->__ctype_encoding;
		const char *numeric = (loc->_numeric_using_locale ? loc->__lc_numeric->_numeric_locale_buf : C);
		if (strcmp(ctype, numeric) == 0) {
			loc->__numeric_fp_cvt = LC_NUMERIC_FP_SAME_LOCALE;
			return loc;
		} else {
			loc->__lc_numeric_loc = newlocale(LC_CTYPE_MASK, numeric, (locale_t)&__c_locale);
			if (loc->__lc_numeric_loc) {
				loc->__numeric_fp_cvt = LC_NUMERIC_FP_USE_LOCALE;
				return loc->__lc_numeric_loc;
			} else { /* shouldn't happen, but just use the same locale */
				loc->__numeric_fp_cvt = LC_NUMERIC_FP_SAME_LOCALE;
				return loc;
			}
		}
	}
	case LC_NUMERIC_FP_SAME_LOCALE:
		return loc;
	case LC_NUMERIC_FP_USE_LOCALE:
		return loc->__lc_numeric_loc;
	}
	return loc;	/* shouldn't happen */
}

/*
 * EXTERNAL: Returns the locale string for the part specified in mask.  The
 * least significant bit is used.  If loc is NULL, the current per-thread
 * locale is used.
 */
const char *
querylocale(int mask, locale_t loc)
{
	int m;
	const char *ret;

	if (_checklocale(loc) < 0 || (mask & LC_ALL_MASK) == 0) {
		errno = EINVAL;
		return NULL;
	}
	DEFAULT_CURRENT_LOCALE(loc);
	m = ffs(mask);
	if (m == 0 || m > _LC_NUM_MASK) {
		errno = EINVAL;
		return NULL;
	}
	XL_LOCK(loc);
	switch(1 << (m - 1)) {
	case LC_COLLATE_MASK:
		ret = (loc->__collate_load_error ? C : loc->__lc_collate->__encoding);
		break;
	case LC_CTYPE_MASK:
		ret = loc->__lc_ctype->__ctype_encoding;
		break;
	case LC_MESSAGES_MASK:
		ret = (loc->_messages_using_locale ? loc->__lc_messages->_messages_locale_buf : C);
		break;
	case LC_MONETARY_MASK:
		ret = (loc->_monetary_using_locale ? loc->__lc_monetary->_monetary_locale_buf : C);
		break;
	case LC_NUMERIC_MASK:
		ret = (loc->_numeric_using_locale ? loc->__lc_numeric->_numeric_locale_buf : C);
		break;
	case LC_TIME_MASK:
		ret = (loc->_time_using_locale ? loc->__lc_time->_time_locale_buf : C);
		break;
	default:
		/* should never get here */
		XL_UNLOCK(loc);
		errno = EINVAL;
		return NULL;
	}
	XL_UNLOCK(loc);
	return ret;
}

/*
 * EXTERNAL: Set the thread-specific locale.  The previous locale is returned.
 * Use LC_GLOBAL_LOCALE to set the global locale.  LC_GLOBAL_LOCALE
 * may also be returned if there was no previous thread-specific locale in
 * effect.  If loc is NULL, the current locale is returned, but no locale
 * chance is made.  NULL is returned on error.
 */
locale_t
uselocale(locale_t loc)
{
	locale_t orig;

	if (loc == NULL)
		orig = (locale_t)pthread_getspecific(__locale_key);
	else {
		if (_checklocale(loc) < 0) {
			errno = EINVAL;
			return NULL;
		}
		if (loc == LC_GLOBAL_LOCALE ||
		    loc == &__global_locale)	/* should never happen */
			loc = NULL;
		XL_RETAIN(loc);
		orig = pthread_getspecific(__locale_key);
		pthread_setspecific(__locale_key, loc);
		XL_RELEASE(orig);
	}
	return (orig ? orig : LC_GLOBAL_LOCALE);
}

/*
 * EXTERNAL: Used by the MB_CUR_MAX macro to determine the thread-specific
 * value.
 */
int
___mb_cur_max(void)
{
	return __current_locale()->__lc_ctype->__mb_cur_max;
}

/*
 * EXTERNAL: Used by the MB_CUR_MAX_L macro to determine the thread-specific
 * value, from the given locale_t.
 */
int
___mb_cur_max_l(locale_t loc)
{
	return __locale_ptr(loc)->__lc_ctype->__mb_cur_max;
}

static void
__xlocale_release(void *loc)
{
	locale_t l = loc;
	XL_RELEASE(l);
}

/*
 * Called from the Libc initializer to setup the thread-specific key.
 */
__private_extern__ void
__xlocale_init(void)
{
	if (__locale_key == (pthread_key_t)-1) {
		__locale_key = __LIBC_PTHREAD_KEY_XLOCALE;
		pthread_key_init_np(__locale_key, __xlocale_release);
	}
}

