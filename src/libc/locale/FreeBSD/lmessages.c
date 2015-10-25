/*
 * Copyright (c) 2001 Alexey Zelkin <phantom@FreeBSD.org>
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
__FBSDID("$FreeBSD: src/lib/libc/locale/lmessages.c,v 1.14 2003/06/26 10:46:16 phantom Exp $");

#include "xlocale_private.h"

#include <stddef.h>
#include <string.h>

#include "ldpart.h"
#include "lmessages.h"

#define LCMESSAGES_SIZE_FULL (sizeof(struct lc_messages_T) / sizeof(char *))
#define LCMESSAGES_SIZE_MIN \
		(offsetof(struct lc_messages_T, yesstr) / sizeof(char *))

static char empty[] = "";

static const struct lc_messages_T _C_messages_locale = {
	"^[yY]" ,	/* yesexpr */
	"^[nN]" ,	/* noexpr */
	"yes" , 	/* yesstr */
	"no"		/* nostr */
};

__private_extern__ int
__messages_load_locale(const char *name, locale_t loc)
{
	int ret;
	struct __xlocale_st_messages *xp;
	static struct __xlocale_st_messages *cache = NULL;

	/* 'name' must be already checked. */
	if (strcmp(name, "C") == 0 || strcmp(name, "POSIX") == 0) {
		loc->_messages_using_locale = 0;
		XL_RELEASE(loc->__lc_messages);
		loc->__lc_messages = NULL;
		return (_LDP_CACHE);
	}

	/*
	 * If the locale name is the same as our cache, use the cache.
	 */
	if (cache && cache->_messages_locale_buf && strcmp(name, cache->_messages_locale_buf) == 0) {
		loc->_messages_using_locale = 1;
		XL_RELEASE(loc->__lc_messages);
		loc->__lc_messages = cache;
		XL_RETAIN(loc->__lc_messages);
		return (_LDP_CACHE);
	}
	if ((xp = (struct __xlocale_st_messages *)malloc(sizeof(*xp))) == NULL)
		return _LDP_ERROR;
	xp->__refcount = 1;
	xp->__free_extra = (__free_extra_t)__ldpart_free_extra;
	xp->_messages_locale_buf = NULL;

	ret = __part_load_locale(name, &loc->_messages_using_locale,
		  &xp->_messages_locale_buf, "LC_MESSAGES/LC_MESSAGES",
		  LCMESSAGES_SIZE_FULL, LCMESSAGES_SIZE_MIN,
		  (const char **)&xp->_messages_locale);
	if (ret == _LDP_LOADED) {
		if (xp->_messages_locale.yesstr == NULL)
			xp->_messages_locale.yesstr = empty;
		if (xp->_messages_locale.nostr == NULL)
			xp->_messages_locale.nostr = empty;
		XL_RELEASE(loc->__lc_messages);
		loc->__lc_messages = xp;
		XL_RELEASE(cache);
		cache = xp;
		XL_RETAIN(cache);
	} else if (ret == _LDP_ERROR)
		free(xp);
	return (ret);
}

__private_extern__ struct lc_messages_T *
__get_current_messages_locale(locale_t loc)
{
	return (loc->_messages_using_locale
		? &loc->__lc_messages->_messages_locale
		: (struct lc_messages_T *)&_C_messages_locale);
}

#ifdef LOCALE_DEBUG
void
msgdebug() {
locale_t loc = __current_locale();
printf(	"yesexpr = %s\n"
	"noexpr = %s\n"
	"yesstr = %s\n"
	"nostr = %s\n",
	loc->__lc_messages->_messages_locale.yesexpr,
	loc->__lc_messages->_messages_locale.noexpr,
	loc->__lc_messages->_messages_locale.yesstr,
	loc->__lc_messages->_messages_locale.nostr
);
}
#endif /* LOCALE_DEBUG */
