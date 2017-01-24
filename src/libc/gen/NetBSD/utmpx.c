/*	$NetBSD: utmpx.c,v 1.25 2008/04/28 20:22:59 martin Exp $	 */

/*-
 * Copyright (c) 2002 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Christos Zoulas.
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
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <sys/cdefs.h>

#if defined(LIBC_SCCS) && !defined(lint)
__RCSID("$NetBSD: utmpx.c,v 1.25 2008/04/28 20:22:59 martin Exp $");
#endif /* LIBC_SCCS and not lint */

#include "namespace.h"
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef UNIFDEF_LEGACY_UTMP_APIS
#include <utmp.h>
#endif /* UNIFDEF_LEGACY_UTMP_APIS */
#include <utmpx.h>
#include <utmpx-darwin.h>
#include <errno.h>
#include <vis.h>
#include <notify.h>

static struct _utmpx *__utx__ = NULL;

static void
__default_utx_init(void)
{
	__utx__ = calloc(1, sizeof(struct _utmpx));
	const char magic[] = __UTX_MAGIC__;
	memcpy(&__utx__->magic, magic, UTMPX_MAGIC);
	pthread_mutex_init(&__utx__->utmpx_mutex, NULL);
	__utx__->utfile = _PATH_UTMPX;
	__utx__->utfile_system = 1;
}

struct _utmpx *
__default_utx(void)
{
	static pthread_once_t once = PTHREAD_ONCE_INIT;
	pthread_once(&once, &__default_utx_init);
	return __utx__;
}

static struct utmpx *__getutxid(struct _utmpx *, const struct utmpx *);

__private_extern__ const char _utmpx_vers[] = "utmpx-1.00";

__private_extern__ void
__setutxent(struct _utmpx *U)
{

	(void)memset(&U->ut, 0, sizeof(U->ut));
	if (U->fp == NULL)
		return;
#ifdef __LP64__
	(void)fseeko(U->fp, (off_t)sizeof(struct utmpx32), SEEK_SET);
#else /* __LP64__ */
	(void)fseeko(U->fp, (off_t)sizeof(U->ut), SEEK_SET);
#endif /* __LP64__ */
}

void
_setutxent(struct _utmpx *U)
{

	TEST_UTMPX_T("_setutxent", U);
	UTMPX_LOCK(U);
	__setutxent(U);
	UTMPX_UNLOCK(U);
}


void
setutxent(void)
{
	_setutxent(__default_utx());
}


__private_extern__ void
__endutxent(struct _utmpx *U)
{
	(void)memset(&U->ut, 0, sizeof(U->ut));
	if (U->fp != NULL) {
		int saveerrno = errno;
		(void)fclose(U->fp);
		errno = saveerrno;
		U->fp = NULL;
		U->readonly = 0;
	}
}


void
_endutxent(struct _utmpx *U)
{
	TEST_UTMPX_T("_endutxent", U);
	UTMPX_LOCK(U);
	__endutxent(U);
	UTMPX_UNLOCK(U);
}


void
endutxent(void)
{
	_endutxent(__default_utx());
}


__private_extern__ struct utmpx *
__getutxent(struct _utmpx *U)
{
	int saveerrno;
#ifdef __LP64__
	struct utmpx32 ut32;
#endif /* __LP64__ */

	if (U->fp == NULL) {
		struct stat st;

		if ((U->fp = fopen(U->utfile, "r+")) == NULL)
			if ((U->fp = fopen(U->utfile, "w+")) == NULL) {
				if ((U->fp = fopen(U->utfile, "r")) == NULL)
					goto fail;
				else
					U->readonly = 1;
			}

		fcntl(fileno(U->fp), F_SETFD, 1); /* set close-on-exec flag */

		/* get file size in order to check if new file */
		if (fstat(fileno(U->fp), &st) == -1)
			goto failclose;

		if (st.st_size == 0) {
			/* new file, add signature record */
#ifdef __LP64__
			(void)memset(&ut32, 0, sizeof(ut32));
			ut32.ut_type = SIGNATURE;
			(void)memcpy(ut32.ut_user, _utmpx_vers, sizeof(_utmpx_vers));
			if (fwrite(&ut32, sizeof(ut32), 1, U->fp) != 1)
#else /* __LP64__ */
			(void)memset(&U->ut, 0, sizeof(U->ut));
			U->ut.ut_type = SIGNATURE;
			(void)memcpy(U->ut.ut_user, _utmpx_vers, sizeof(_utmpx_vers));
			if (fwrite(&U->ut, sizeof(U->ut), 1, U->fp) != 1)
#endif /* __LP64__ */
				goto failclose;
		} else {
			/* old file, read signature record */
#ifdef __LP64__
			if (fread(&ut32, sizeof(ut32), 1, U->fp) != 1)
#else /* __LP64__ */
			if (fread(&U->ut, sizeof(U->ut), 1, U->fp) != 1)
#endif /* __LP64__ */
				goto failclose;
#ifdef __LP64__
			if (memcmp(ut32.ut_user, _utmpx_vers, sizeof(_utmpx_vers)) != 0 ||
			    ut32.ut_type != SIGNATURE)
#else /* __LP64__ */
			if (memcmp(U->ut.ut_user, _utmpx_vers, sizeof(_utmpx_vers)) != 0 ||
			    U->ut.ut_type != SIGNATURE)
#endif /* __LP64__ */
			{
				errno = EINVAL;
				goto failclose;
			}
		}
	}

#ifdef __LP64__
	if (fread(&ut32, sizeof(ut32), 1, U->fp) != 1)
#else /* __LP64__ */
	if (fread(&U->ut, sizeof(U->ut), 1, U->fp) != 1)
#endif /* __LP64__ */
		goto fail;

#ifdef __LP64__
	_utmpx32_64(&ut32, &U->ut);
#endif /* __LP64__ */
	return &U->ut;
failclose:
	saveerrno = errno;
	(void)fclose(U->fp);
	errno = saveerrno;
	U->fp = NULL;
fail:
	(void)memset(&U->ut, 0, sizeof(U->ut));
	return NULL;
}


struct utmpx *
_getutxent(struct _utmpx *U)
{
	struct utmpx *ret;

	TEST_UTMPX_T("_getutxent", U);
	UTMPX_LOCK(U);
	ret = __getutxent(U);
	UTMPX_UNLOCK(U);
	return ret;
}


struct utmpx *
getutxent(void)
{
	return _getutxent(__default_utx());
}


struct utmpx *
_getutxid(struct _utmpx *U, const struct utmpx *utx)
{
	struct utmpx temp;
	const struct utmpx *ux;
	struct utmpx *ret;

	if (utx->ut_type == EMPTY)
		return NULL;

	TEST_UTMPX_T("_getutxid", U);
	UTMPX_LOCK(U);
	/* make a copy as needed, and auto-fill if requested */
	ux = _utmpx_working_copy(utx, &temp, 1);
	if (!ux) {
		UTMPX_UNLOCK(U);
		return NULL;
	}

	ret = __getutxid(U, ux);
	UTMPX_UNLOCK(U);
	return ret;
}


struct utmpx *
getutxid(const struct utmpx *utx)
{
	return _getutxid(__default_utx(), utx);
}


static struct utmpx *
__getutxid(struct _utmpx *U, const struct utmpx *utx)
{

	do {
		if (U->ut.ut_type == EMPTY)
			continue;
		switch (utx->ut_type) {
		case EMPTY:
			return NULL;
		case RUN_LVL:
		case BOOT_TIME:
		case OLD_TIME:
		case NEW_TIME:
			if (U->ut.ut_type == utx->ut_type)
				return &U->ut;
			break;
		case INIT_PROCESS:
		case LOGIN_PROCESS:
		case USER_PROCESS:
		case DEAD_PROCESS:
			switch (U->ut.ut_type) {
			case INIT_PROCESS:
			case LOGIN_PROCESS:
			case USER_PROCESS:
			case DEAD_PROCESS:
				if (memcmp(U->ut.ut_id, utx->ut_id,
				    sizeof(U->ut.ut_id)) == 0)
					return &U->ut;
				break;
			default:
				break;
			}
			break;
		default:
			return NULL;
		}
	} while (__getutxent(U) != NULL);
	return NULL;
}


static struct utmpx *
__getutxline(struct _utmpx *U, const struct utmpx *utx)
{
	do {
		switch (U->ut.ut_type) {
		case EMPTY:
			break;
		case LOGIN_PROCESS:
		case USER_PROCESS:
			if (strncmp(U->ut.ut_line, utx->ut_line,
			    sizeof(U->ut.ut_line)) == 0)
				return &U->ut;
			break;
		default:
			break;
		}
	} while (__getutxent(U) != NULL);
	return NULL;
}


struct utmpx *
_getutxline(struct _utmpx *U, const struct utmpx *utx)
{
	struct utmpx *ret;

	TEST_UTMPX_T("_getutxline", U);
	UTMPX_LOCK(U);
	ret = __getutxline(U, utx);
	UTMPX_UNLOCK(U);
	return ret;
}


struct utmpx *
getutxline(const struct utmpx *utx)
{
	return _getutxline(__default_utx(), utx);
}


struct utmpx *
_pututxline(struct _utmpx *U, const struct utmpx *utx)
{
	struct utmpx *ux;

	if (utx == NULL) {
		errno = EINVAL;
		return NULL;
	}

	TEST_UTMPX_T("_pututxline", U);
	UTMPX_LOCK(U);
	if ((ux = __pututxline(__default_utx(), utx)) != NULL && __default_utx()->utfile_system) {
		_utmpx_asl(ux);	/* the equivalent of wtmpx and lastlogx */
#ifdef UTMP_COMPAT
		_write_utmp_compat(ux);
#endif /* UTMP_COMPAT */
	}
	UTMPX_UNLOCK(U);
	return ux;
}


struct utmpx *
pututxline(const struct utmpx *utx)
{
	return _pututxline(__default_utx(), utx);
}

__private_extern__ struct utmpx *
__pututxline(struct _utmpx *U, const struct utmpx *utx)
{
	struct utmpx temp, *u = NULL, *x;
	const struct utmpx *ux;
#ifdef __LP64__
	struct utmpx32 ut32;
#endif /* __LP64__ */
	struct flock fl;
#define gotlock		(fl.l_start >= 0)

	fl.l_start = -1; /* also means we haven't locked */
	if (U->utfile_system)
		if ((U->fp != NULL && U->readonly) || (U->fp == NULL && geteuid() != 0)) {
			errno = EPERM;
			return NULL;
		}

	if (U->fp == NULL) {
		(void)__getutxent(U);
		if (U->fp == NULL || U->readonly) {
			errno = EPERM;
			return NULL;
		}
	}

	/* make a copy as needed, and auto-fill if requested */
	ux = _utmpx_working_copy(utx, &temp, 0);
	if (!ux)
		return NULL;

	if ((x = __getutxid(U, ux)) == NULL) {
		__setutxent(U);
		if ((x = __getutxid(U, ux)) == NULL) {
			/*
			 * utx->ut_type has any original mask bits, while
			 * ux->ut_type has those mask bits removed.  If we
			 * are trying to record a dead process, and
			 * UTMPX_DEAD_IF_CORRESPONDING_MASK is set, then since
			 * there is no matching entry, we return NULL.
			 */
			if (ux->ut_type == DEAD_PROCESS &&
			    (utx->ut_type & UTMPX_DEAD_IF_CORRESPONDING_MASK)) {
				errno = EINVAL;
				return NULL;
			}
			/*
			 * Replace lockf() with fcntl() and a fixed start
			 * value.  We should already be at EOF.
			 */
			if ((fl.l_start = lseek(fileno(U->fp), 0, SEEK_CUR)) < 0)
				return NULL;
			fl.l_len = 0;
			fl.l_whence = SEEK_SET;
			fl.l_type = F_WRLCK;
			if (fcntl(fileno(U->fp), F_SETLKW, &fl) == -1)
				return NULL;
			if (fseeko(U->fp, (off_t)0, SEEK_END) == -1)
				goto fail;
		}
	}

	if (!gotlock) {
		/*
		 * utx->ut_type has any original mask bits, while
		 * ux->ut_type has those mask bits removed.  If we
		 * are trying to record a dead process, if
		 * UTMPX_DEAD_IF_CORRESPONDING_MASK is set, but the found
		 * entry is not a (matching) USER_PROCESS, then return NULL.
		 */
		if (ux->ut_type == DEAD_PROCESS &&
		    (utx->ut_type & UTMPX_DEAD_IF_CORRESPONDING_MASK) &&
		    x->ut_type != USER_PROCESS) {
			errno = EINVAL;
			return NULL;
		}
		/* we are not appending */
#ifdef __LP64__
		if (fseeko(U->fp, -(off_t)sizeof(ut32), SEEK_CUR) == -1)
#else /* __LP64__ */
		if (fseeko(U->fp, -(off_t)sizeof(U->ut), SEEK_CUR) == -1)
#endif /* __LP64__ */
			return NULL;
	}

#ifdef __LP64__
	_utmpx64_32(ux, &ut32);
	if (fwrite(&ut32, sizeof (ut32), 1, U->fp) != 1)
#else /* __LP64__ */
	if (fwrite(ux, sizeof (*ux), 1, U->fp) != 1)
#endif /* __LP64__ */
		goto fail;

	if (fflush(U->fp) == -1)
		goto fail;

	u = memcpy(&U->ut, ux, sizeof(U->ut));
	notify_post(UTMPX_CHANGE_NOTIFICATION);
fail:
	if (gotlock) {
		int save = errno;
		fl.l_type = F_UNLCK;
		if (fcntl(fileno(U->fp), F_SETLK, &fl) == -1)
			return NULL;
		errno = save;
	}
	return u;
}


/*
 * The following are extensions and not part of the X/Open spec.
 */
__private_extern__ int
__utmpxname(struct _utmpx *U, const char *fname)
{
	size_t len;

	if (fname == NULL) {
		if(!U->utfile_system)
			free(U->utfile);
		U->utfile = _PATH_UTMPX;
		U->utfile_system = 1;
		__endutxent(U);
		return 1;
	}

	len = strlen(fname);

	if (len >= MAXPATHLEN)
		return 0;

	/* must end in x! */
	if (fname[len - 1] != 'x')
		return 0;

	if (U->utfile_system)
		U->utfile = NULL;
	U->utfile_system = 0;
	if ((U->utfile = reallocf(U->utfile, len + 1)) == NULL)
		return 0;

	(void)strcpy(U->utfile, fname);
	__endutxent(U);
	return 1;
}

int
_utmpxname(struct _utmpx *U, const char *fname)
{
	int ret;

	TEST_UTMPX_T("_utmpxname", U);
	UTMPX_LOCK(U);
	ret = __utmpxname(U, fname);
	UTMPX_UNLOCK(U);
	return ret;
}

int
utmpxname(const char *fname)
{
	return _utmpxname(__default_utx(), fname);
}

#ifdef UNIFDEF_LEGACY_UTMP_APIS
void
getutmp(const struct utmpx *ux, struct utmp *u)
{

	bzero(u, sizeof(*u));
	(void)memcpy(u->ut_name, ux->ut_user, sizeof(u->ut_name));
	(void)memcpy(u->ut_line, ux->ut_line, sizeof(u->ut_line));
	(void)memcpy(u->ut_host, ux->ut_host, sizeof(u->ut_host));
	u->ut_time = ux->ut_tv.tv_sec;
}

void
getutmpx(const struct utmp *u, struct utmpx *ux)
{

	bzero(ux, sizeof(*ux));
	(void)memcpy(ux->ut_user, u->ut_name, sizeof(u->ut_name));
	ux->ut_user[sizeof(u->ut_name)] = 0;
	(void)memcpy(ux->ut_line, u->ut_line, sizeof(u->ut_line));
	ux->ut_line[sizeof(u->ut_line)] = 0;
	(void)memcpy(ux->ut_host, u->ut_host, sizeof(u->ut_host));
	ux->ut_host[sizeof(u->ut_host)] = 0;
	ux->ut_tv.tv_sec = u->ut_time;
	ux->ut_tv.tv_usec = 0;
	ux->ut_pid = getpid();
	ux->ut_type = USER_PROCESS;
}
#endif /* UNIFDEF_LEGACY_UTMP_APIS */
