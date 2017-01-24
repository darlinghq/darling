/*
 * Copyright (c) 2005-2009 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * "Portions Copyright (c) 2004 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.0 (the 'License').  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License."
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef UTMP_COMPAT
#include <utmp.h>
#endif /* UTMP_COMPAT */
#include <utmpx.h>
#include <utmpx-darwin.h>
#include <utmpx_thread.h>
#include <asl.h>
#include <pwd.h>
#include <stddef.h>

#include <mach/mach.h>
#include <mach/std_types.h>
#include <mach/mig.h>
#include <mach/mach_types.h>
#include <servers/bootstrap.h>
#include <pthread.h>

#ifdef UTMP_COMPAT
#include <ttyent.h>
#endif /* UTMP_COMPAT */

#if ASL_API_VERSION < 20131108
#include <asl_private.h>
#include <asl_store.h>
#endif

__private_extern__ const char __utx_magic__[UTMPX_MAGIC] = __UTX_MAGIC__;

extern const char _utmpx_vers[];	/* in utmpx.c */

#if ASL_API_VERSION < 20131108
static void msg2lastlogx(const aslmsg, struct lastlogx *);
static void msg2utmpx(const aslmsg, struct utmpx *);
static void utmpx2msg(const struct utmpx *, aslmsg);
#else
static void msg2lastlogx(asl_object_t, struct lastlogx *);
static void msg2utmpx(asl_object_t, struct utmpx *);
static void utmpx2msg(const struct utmpx *, asl_object_t);
#endif

static size_t pw_size = 0;

#define FACILITY		"Facility"
#define WTMP_COUNT		32

/* ASL timeout in microseconds */
#define ASL_QUERY_TIMEOUT 4000000

/* indirection causes argument to be substituted before stringification */
#define STR(x)		__STRING(x)

#ifdef UTMP_COMPAT
static char *
_pwnam_r(const char *user, struct passwd *pw)
{
	struct passwd *p;
	char *buf;

	if (pw_size <= 0) {
		pw_size = sysconf(_SC_GETPW_R_SIZE_MAX);
		if (pw_size <= 0)
			return NULL;
	}
	if ((buf = malloc(pw_size)) == NULL)
		return NULL;

	getpwnam_r(user, pw, buf, pw_size, &p);
	if (!p) {
		free(buf);
		return NULL;
	}
	return buf;
}
#endif

static char *
_pwuid_r(uid_t uid, struct passwd *pw)
{
	struct passwd *p;
	char *buf;

	if (pw_size <= 0) {
		pw_size = sysconf(_SC_GETPW_R_SIZE_MAX);
		if (pw_size <= 0)
			return NULL;
	}
	if ((buf = malloc(pw_size)) == NULL)
		return NULL;

	getpwuid_r(uid, pw, buf, pw_size, &p);
	if (!p) {
		free(buf);
		return NULL;
	}
	return buf;
}

struct lastlogx *
getlastlogx(uid_t uid, struct lastlogx *lx)
{
	char *buf;
	struct passwd pw;
	struct lastlogx *l;

	if ((buf = _pwuid_r(uid, &pw)) == NULL)
		return NULL;

	l = getlastlogxbyname(pw.pw_name, lx);
	free(buf);
	return l;
}

struct lastlogx *
getlastlogxbyname(const char *user, struct lastlogx *lx)
{
#if ASL_API_VERSION < 20131108
	aslmsg m;
	asl_msg_t *qm[1];
	asl_search_result_t query, *res;
	uint32_t status;
	asl_store_t *store = NULL;
	uint64_t cmax;
#else
	asl_object_t m, query, res;
	size_t cmax;
#endif
	struct lastlogx *result = NULL;

	if (!user || !*user) return NULL;

#if ASL_API_VERSION < 20131108
	status = asl_store_open_read(NULL, &store);
	if (status != 0) return NULL;
	if (store == NULL) return NULL;
#endif

	/*
	 * We search for the last LASTLOG_FACILITY entry that has the
	 * ut_user entry matching the user's name.
	 */
	m = asl_new(ASL_TYPE_QUERY);
	if (m == NULL)
	{
#if ASL_API_VERSION < 20131108
		asl_store_close(store);
#endif
		return NULL;
	}

	asl_set_query(m, FACILITY, LASTLOG_FACILITY, ASL_QUERY_OP_EQUAL);
	asl_set_query(m, "ut_user", user, ASL_QUERY_OP_EQUAL);

#if ASL_API_VERSION < 20131108
	qm[0] = (asl_msg_t *)m;
	query.count = 1;
	query.msg = qm;
#else
	query = asl_new(ASL_TYPE_LIST);
	if (query == NULL)
	{
		asl_release(m);
		return NULL;
	}

	asl_append(query, m);
	asl_release(m);
#endif

	res = NULL;
	cmax = 0;

#if ASL_API_VERSION < 20131108
	asl_store_match_timeout(store, &query, &res, &cmax, -1, 1, -1, ASL_QUERY_TIMEOUT);
	asl_store_close(store);
	asl_free(m);
#else
	res = asl_match(NULL, query, &cmax, -1, 1, ASL_QUERY_TIMEOUT, ASL_MATCH_DIRECTION_REVERSE);
	asl_release(query);
#endif

	if (res == NULL) return NULL;

#if ASL_API_VERSION < 20131108
	m = aslresponse_next(res);
#else
	m = asl_next(res);
#endif

	if (m == NULL)
	{
#if ASL_API_VERSION < 20131108
		aslresponse_free(res);
#else
		asl_release(res);
#endif
		return NULL;
	}

	if (lx == NULL)
	{
		if ((lx = (struct lastlogx *)malloc(sizeof(*lx))) == NULL)
		{
#if ASL_API_VERSION < 20131108
			aslresponse_free(res);
#else
			asl_release(res);
#endif
			return NULL;
		}
	}

	msg2lastlogx(m, lx);
#if ASL_API_VERSION < 20131108
	aslresponse_free(res);
#else
	asl_release(res);
#endif
	result = lx;

	return result;
}

#define IGET(e,p)	if ((cp = asl_get(m, __STRING(ut_##e))) != NULL) \
				u->p##_##e = strtol(cp, NULL, 10);
#define LGET(e,p)	IGET(e,p)
#define SGET(e,p)	if ((cp = asl_get(m, __STRING(ut_##e))) != NULL) \
				strncpy(u->p##_##e, cp, sizeof(u->p##_##e))

/* fill in a struct lastlogx from an ASL message */
static void
#if ASL_API_VERSION < 20131108
msg2lastlogx(const aslmsg m, struct lastlogx *u)
#else
msg2lastlogx(asl_object_t m, struct lastlogx *u)
#endif
{
	const char *cp;

	bzero(u, sizeof(*u));
	SGET(line, ll);
	LGET(tv.tv_sec, ll);
	IGET(tv.tv_usec, ll);
	SGET(host, ll);
}

/* fill in a struct utmpx from an ASL message */
static void
#if ASL_API_VERSION < 20131108
msg2utmpx(aslmsg m, struct utmpx *u)
#else
msg2utmpx(asl_object_t m, struct utmpx *u)
#endif
{
	const char *cp;

	bzero(u, sizeof(*u));
	SGET(user, ut);
	SGET(id, ut);
	SGET(line, ut);
	IGET(pid, ut);
	IGET(type, ut);
	LGET(tv.tv_sec, ut);
	IGET(tv.tv_usec, ut);
	SGET(host, ut);
}

/* fill in an ASL message from a struct utmpx */
static void
#if ASL_API_VERSION < 20131108
utmpx2msg(const struct utmpx *u, aslmsg m)
#else
utmpx2msg(const struct utmpx *u, asl_object_t m)
#endif
{
	char buf[_UTX_HOSTSIZE + 1];	/* the largest string in struct utmpx */
	const char *cp;
#define ISET(e)	{ snprintf(buf, sizeof(buf), "%d", u->e); \
		asl_set(m, #e, buf); }
#define LSET(e)	{ snprintf(buf, sizeof(buf), "%ld", u->e); \
		asl_set(m, #e, buf); }
#define SSET(e)	if (*(u->e)) { \
			strncpy(buf, u->e, sizeof(u->e)); \
			buf[sizeof(u->e)] = 0; \
			asl_set(m, #e, buf); \
		}

	SSET(ut_user);
	cp = (char *)u->ut_id + sizeof(u->ut_id);
	while(--cp >= u->ut_id && isprint(*cp)) {}
	if(cp < u->ut_id) {
	    SSET(ut_id);
	} else {
	    snprintf(buf, sizeof(buf), "0x%02x 0x%02x 0x%02x 0x%02x",
		(unsigned)u->ut_id[0], (unsigned)u->ut_id[1],
		(unsigned)u->ut_id[2], (unsigned)u->ut_id[3]);
	    asl_set(m, "ut_id", buf);
	}
	SSET(ut_line);
	if (u->ut_pid > 0)
		ISET(ut_pid);
	ISET(ut_type);
	LSET(ut_tv.tv_sec);
	ISET(ut_tv.tv_usec);
	SSET(ut_host);
}

static const char *utmpx_types[] = {
	"EMPTY",		/* 0 */
	"RUN_LVL",		/* 1 */
	"BOOT_TIME",		/* 2 */
	"OLD_TIME",		/* 3 */
	"NEW_TIME",		/* 4 */
	"INIT_PROCESS",		/* 5 */
	"LOGIN_PROCESS",	/* 6 */
	"USER_PROCESS",		/* 7 */
	"DEAD_PROCESS",		/* 8 */
	"ACCOUNTING",		/* 9 */
	"SIGNATURE",		/* 10 */
	"SHUTDOWN_TIME",	/* 11 */
};

/* send a struct utmpx record using ASL */
__private_extern__ void
_utmpx_asl(const struct utmpx *u)
{
#if ASL_API_VERSION < 20131108
	aslclient asl = asl_open(NULL, NULL, ASL_OPT_NO_REMOTE); /* could be NULL, but still works */
	aslmsg m;
#else
	asl_object_t asl = asl_open(NULL, NULL, ASL_OPT_NO_REMOTE);
	asl_object_t m;
#endif
	char msg[64];

	if (u->ut_type == EMPTY)
		return;
	if ((m = asl_new(ASL_TYPE_MSG)) == NULL) {
#if ASL_API_VERSION < 20131108
		asl_close(asl);
#else
		asl_release(asl);
#endif
		return;
	}
	/*
	 * If the ut_type is USER_PROCESS, we use the LASTLOG_FACILITY,
	 * otherwise we use the UTMPX_FACILITY.  This makes it easy to
	 * search for lastlog entries, but for wtmp, we have to search
	 * for both facilities.
	 */
	if (u->ut_type == USER_PROCESS)
		asl_set(m, FACILITY, LASTLOG_FACILITY);
	else
		asl_set(m, FACILITY, UTMPX_FACILITY);
	asl_set(m, ASL_KEY_LEVEL, STR(ASL_LEVEL_NOTICE));
	utmpx2msg(u, m);

	/* Make a visible message for system.log */
	switch (u->ut_type) {
	case BOOT_TIME:
	case OLD_TIME:
	case NEW_TIME:
	case SHUTDOWN_TIME:
		sprintf(msg, "%s: %ld %d", utmpx_types[u->ut_type], u->ut_tv.tv_sec, u->ut_tv.tv_usec);
		break;
	case INIT_PROCESS:
	case LOGIN_PROCESS:
		sprintf(msg, "%s: %d", utmpx_types[u->ut_type], (int)u->ut_pid);
		break;
	case USER_PROCESS:
	case DEAD_PROCESS:
		sprintf(msg, "%s: %d %.*s", utmpx_types[u->ut_type], (int)u->ut_pid, (int)sizeof(u->ut_line), u->ut_line);
		break;
	default:
		if (u->ut_type >= 0 && u->ut_type < (sizeof(utmpx_types) / sizeof(*utmpx_types)))
			sprintf(msg, "%s", utmpx_types[u->ut_type]);
		else
			sprintf(msg, "ut_type=%d", (int)u->ut_type);
		break;
	}
	asl_set(m, ASL_KEY_MSG, msg);
	asl_send(asl, m);
#if ASL_API_VERSION < 20131108
	asl_free(m);
	asl_close(asl);
#else
	asl_release(m);
	asl_release(asl);
#endif
}

#define UT_USER	(1 << 0)
#define UT_ID	(1 << 1)
#define UT_LINE	(1 << 2)
#define UT_PID	(1 << 3)
#define UT_TV	(1 << 4)

__private_extern__ const struct utmpx *
_utmpx_working_copy(const struct utmpx *utx, struct utmpx *temp, int onlyid)
{
	int which;
	static char idzero[_UTX_IDSIZE];

	if ((utx->ut_type & (UTMPX_AUTOFILL_MASK | UTMPX_DEAD_IF_CORRESPONDING_MASK)) == 0)
		return utx;
	memcpy(temp, utx, sizeof(*temp));
	temp->ut_type &= ~(UTMPX_AUTOFILL_MASK | UTMPX_DEAD_IF_CORRESPONDING_MASK);

	if ((utx->ut_type & UTMPX_AUTOFILL_MASK) == 0)
		return temp;

	which = UT_TV; /* they all need time */
	switch(temp->ut_type) {
	case EMPTY:
		return temp;
	case USER_PROCESS:
		which |= (UT_USER | UT_LINE | UT_PID);
		/* Set UT_ID if ut_id isn't there */
		if (memcmp(temp->ut_id, idzero, sizeof(temp->ut_id)) == 0)
			which |= UT_ID;
		break;
	case INIT_PROCESS:
		which |= UT_PID;
		break;
	case LOGIN_PROCESS:
		which |= (UT_USER | UT_PID);
		break;
	case DEAD_PROCESS:
		which |= UT_PID;
		/* Set UT_ID if ut_id isn't there.  We will also need UT_LINE */
		if (memcmp(temp->ut_id, idzero, sizeof(temp->ut_id)) == 0)
			which |= (UT_ID | UT_LINE);
		break;
	}
	/*
	 * If onlyid is set: if ut_id isn't set but is needed, then set
	 * which to (UT_LINE | UT_ID), otherwise zero
	 */
	if (onlyid)
		which = (which & UT_ID) ? (UT_LINE | UT_ID) : 0;
	if ((which & UT_LINE) && !*temp->ut_line) {
		char buf[256];
		char *cp;
#if __DARWIN_UNIX03
		int err;

		err = ttyname_r(0, buf, sizeof(buf));
		if (err)
			err = ttyname_r(1, buf, sizeof(buf));
		if (err)
			err = ttyname_r(2, buf, sizeof(buf));
		if (err)
			return NULL;
#else /* !__DARWIN_UNIX03 */
		cp = ttyname_r(0, buf, sizeof(buf));
		if (!cp)
			cp = ttyname_r(1, buf, sizeof(buf));
		if (!cp)
			cp = ttyname_r(2, buf, sizeof(buf));
		if (!cp)
			return NULL;
#endif /* __DARWIN_UNIX03 */
		cp = strrchr(buf, '/');
		if (cp)
			cp++;
		else
			cp = buf;
		strncpy(temp->ut_line, cp, sizeof(temp->ut_line));
	}
	/* UT_ID is set only if we already know we need to add it */
	if ((which & UT_ID)) {
		char *cp;
		int i = sizeof(temp->ut_line);

		for(cp = temp->ut_line; i > 0 && *cp; i--)
			cp++;
		i = cp - temp->ut_line;
		if(i >= sizeof(temp->ut_id))
			memcpy(temp->ut_id, cp - sizeof(temp->ut_id), sizeof(temp->ut_id));
		else
			memcpy(temp->ut_id, temp->ut_line, i);
	}
	if ((which & UT_PID) && !temp->ut_pid)
		temp->ut_pid = getpid();
	if ((which & UT_USER) && !*temp->ut_user) {
		char *buf;
		struct passwd pw;

		if ((buf = _pwuid_r(getuid(), &pw)) == NULL)
			return NULL;
		strncpy(temp->ut_user, pw.pw_name, sizeof(temp->ut_user));
		free(buf);
	}
	if ((which & UT_TV) && !temp->ut_tv.tv_sec && !temp->ut_tv.tv_usec)
		gettimeofday(&temp->ut_tv, NULL);
	return temp;
}

/*
 * We can read from either asl or from a file, so we need to switch between
 * the two.
 */
static void end_asl(void);
static void end_file(void);
static struct utmpx *get_asl(void);
static struct utmpx *get_file(void);
static void set_asl(int);
static void set_file(int);

enum {WTMP_ASL, WTMP_FILE};

static struct {
	int which;
	void (*end)(void);
	struct utmpx *(*get)(void);
	void (*set)(int);
} wtmp_func = {
	WTMP_ASL,
	end_asl,
	get_asl,
	set_asl
};

static struct {
	uint64_t start;
	int dir;
#if ASL_API_VERSION < 20131108
	asl_search_result_t *res;
#else
	asl_object_t res;
#endif
	char *str;
	uint32_t len;
	char inited;
	char done;
} wtmp_asl = {-1, 1};
static struct {
	int fd;
	int dir;
	char *file;
	off_t off;
	size_t count;
#ifdef __LP64__
	struct utmpx32 *buf;
	struct utmpx32 *next;
#else /* __LP64__ */
	struct utmpx *buf;
	struct utmpx *next;
#endif /* __LP64__ */
	int left;
} wtmp_file = {-1, -1};

void
endutxent_wtmp(void)
{
	wtmp_func.end();
}

struct utmpx *
getutxent_wtmp(void)
{
	return wtmp_func.get();
}

void
setutxent_wtmp(int dir)
{
	wtmp_func.set(dir);
}

/* use the given file, or if NULL, read from asl */
int
wtmpxname(const char *fname)
{
	size_t len;

	if (fname == NULL) {
		if (wtmp_func.which == WTMP_ASL) {
			end_asl();
			return 1;
		}
		end_file();
		wtmp_func.which = WTMP_ASL;
		wtmp_func.end = end_asl;
		wtmp_func.get = get_asl;
		wtmp_func.set = set_asl;
		return 1;
	}

	len = strlen(fname);
	if (len >= MAXPATHLEN)
		return 0;

	/* must end in x! */
	if (fname[len - 1] != 'x')
		return 0;

	if (wtmp_func.which == WTMP_ASL)
		end_asl();
	else if (wtmp_file.fd >= 0) {
		close(wtmp_file.fd);
		wtmp_file.fd = -1;
	}

	if (wtmp_file.file)
		free(wtmp_file.file);

	wtmp_file.file = strdup(fname);
	if (wtmp_file.file == NULL)
		return 0;

	wtmp_func.which = WTMP_FILE;
	wtmp_func.end = end_file;
	wtmp_func.get = get_file;
	wtmp_func.set = set_file;
	return 1;
}

static void
end_asl(void)
{
	if (wtmp_asl.res != NULL)
	{
#if ASL_API_VERSION < 20131108
		aslresponse_free(wtmp_asl.res);
#else
		asl_release(wtmp_asl.res);
#endif
		wtmp_asl.res = NULL;
	}

	wtmp_asl.inited = 0;
	wtmp_asl.done = 0;
}

static void
end_file(void)
{
	if (wtmp_file.fd >= 0) {
		close(wtmp_file.fd);
		wtmp_file.fd = -1;
	}
	if (wtmp_file.buf) {
		free(wtmp_file.buf);
		wtmp_file.buf = NULL;
	}
}

static struct utmpx *
get_asl(void)
{
#if ASL_API_VERSION < 20131108
	aslmsg m;
#else
	asl_object_t m;
#endif
	static struct utmpx utx;

	if (wtmp_asl.inited == 0) set_asl(-1);
	if (wtmp_asl.done != 0) return NULL;

#if ASL_API_VERSION < 20131108
	m = aslresponse_next(wtmp_asl.res);
#else
	m = asl_next(wtmp_asl.res);
#endif
	if (m == NULL)
	{
#if ASL_API_VERSION < 20131108
		aslresponse_free(wtmp_asl.res);
#else
		asl_release(wtmp_asl.res);
#endif
		wtmp_asl.res = NULL;
		wtmp_asl.done = 1;
		return NULL;
	}

	msg2utmpx(m, &utx);
	return &utx;
}
	

static struct utmpx *
get_file(void)
{
	int n, r;
	char *cp;
#ifdef __LP64__
	static struct utmpx ux;
#endif /* __LP64__ */

get_file_repeat:
	if (wtmp_file.left > 0) {
#ifdef __LP64__
		struct utmpx32 *u = wtmp_file.next;
#else /* __LP64__ */
		struct utmpx *u = wtmp_file.next;
#endif /* __LP64__ */
		wtmp_file.next += wtmp_file.dir;
		wtmp_file.left--;
#ifdef __LP64__
		_utmpx32_64(u, &ux);
		return &ux;
#else /* __LP64__ */
		return u;
#endif /* __LP64__ */
	} else if (wtmp_file.fd < 0) {
		set_file(-1); /* keep current read direction */
		if (wtmp_file.fd < 0)
			return NULL;
		goto get_file_repeat;
	}
	if (wtmp_file.count <= 0)
		return NULL;

#ifdef __LP64__
	n = WTMP_COUNT * sizeof(struct utmpx32);
#else /* __LP64__ */
	n = WTMP_COUNT * sizeof(struct utmpx);
#endif /* __LP64__ */
	if (wtmp_file.dir > 0)
		wtmp_file.next = wtmp_file.buf;
	else {
		wtmp_file.next = wtmp_file.buf + WTMP_COUNT - 1;
		wtmp_file.off -= n;
		if (lseek(wtmp_file.fd, wtmp_file.off, SEEK_SET) < 0) {
get_file_done:
			wtmp_file.count = 0;
			return NULL;
		}
	}

	cp = (char *)wtmp_file.buf;
	do {
		if((r = read(wtmp_file.fd, cp, n)) <= 0) {
			if (r < 0 && (errno == EINTR || errno == EAGAIN))
				continue;
			goto get_file_done;
		}
		cp += r;
	} while((n -= r) > 0);

	wtmp_file.left = WTMP_COUNT;
	wtmp_file.count -= WTMP_COUNT;

	goto get_file_repeat;
}

/*
 * This sets the directions for both asl and reading from a file.  If forward
 * is negative, skip.
 */
static void
_set_dir(int forward)
{
	if (forward < 0) return;

	if (forward == 0)
	{
		/* go backward */
		wtmp_asl.dir = -1;
		wtmp_asl.start = -1;
		wtmp_file.dir = -1;
	}
	else
	{
		/* go forward */
		wtmp_asl.dir = 1;
		wtmp_asl.start = 0;
		wtmp_file.dir = 1;
	}
}

static void
set_asl(int forward)
{
#if ASL_API_VERSION < 20131108
	aslmsg q0, q1;
	asl_msg_t *m[2];
	asl_search_result_t query;
	asl_store_t *store = NULL;
	uint32_t status;
	uint64_t cmax;
#else
	asl_object_t q0, q1, query;
	size_t cmax;
#endif

	_set_dir(forward);

	wtmp_asl.inited = 0;
	wtmp_asl.done = 0;

	if (wtmp_asl.res != NULL)
	{
#if ASL_API_VERSION < 20131108
		aslresponse_free(wtmp_asl.res);
#else
		asl_release(wtmp_asl.res);
#endif
		wtmp_asl.res = NULL;
	}

#if ASL_API_VERSION < 20131108
	status = asl_store_open_read(NULL, &store);
	if (status != 0) return;
	if (store == NULL) return;
#endif

	/*
	 * Create a search query that matches either UTMPX_FACILITY
	 * or LASTLOG_FACILITY.
	 */
	q0 = asl_new(ASL_TYPE_QUERY);
	if (q0 == NULL) return;

	asl_set_query(q0, FACILITY, UTMPX_FACILITY, ASL_QUERY_OP_EQUAL);

	q1 = asl_new(ASL_TYPE_QUERY);
	if (q1 == NULL)
	{
#if ASL_API_VERSION < 20131108
		asl_free(q0);
#else
		asl_release(q0);
#endif
		return;
	}

	asl_set_query(q1, FACILITY, LASTLOG_FACILITY, ASL_QUERY_OP_EQUAL);

#if ASL_API_VERSION < 20131108
	m[0] = (asl_msg_t *)q0;
	m[1] = (asl_msg_t *)q1;
	query.count = 2;
	query.msg = m;
#else
	query = asl_new(ASL_TYPE_LIST);
	if (query == NULL)
	{
		asl_release(q0);
		asl_release(q1);
		return;
	}

	asl_append(query, q0);
	asl_append(query, q1);

	asl_release(q0);
	asl_release(q1);
#endif

	cmax = 0;

#if ASL_API_VERSION < 20131108
	asl_store_match_timeout(store, &query, &(wtmp_asl.res), &cmax, wtmp_asl.start, 0, wtmp_asl.dir, ASL_QUERY_TIMEOUT);
	asl_store_close(store);
	asl_free(q0);
	asl_free(q1);
#else
	wtmp_asl.res = asl_match(NULL, query, &cmax, wtmp_asl.start, 0, ASL_QUERY_TIMEOUT, wtmp_asl.dir);
	asl_release(query);
#endif
	
	if (wtmp_asl.res == NULL) return;

	wtmp_asl.inited = 1;
	wtmp_asl.done = 0;
}

static void
set_file(int forward)
{
	struct stat s;
	size_t c;
	int n, r;
	char *cp;

	_set_dir(forward);
#ifdef __LP64__
	if (wtmp_file.buf == NULL &&
	    (wtmp_file.buf = (struct utmpx32 *)malloc(WTMP_COUNT * sizeof(struct utmpx32))) == NULL)
#else /* __LP64__ */
	if (wtmp_file.buf == NULL &&
	    (wtmp_file.buf = (struct utmpx *)malloc(WTMP_COUNT * sizeof(struct utmpx))) == NULL)
#endif /* __LP64__ */
		return;
	if (wtmp_file.fd >= 0)
		close(wtmp_file.fd);
	if ((wtmp_file.fd = open(wtmp_file.file, O_RDONLY, 0)) < 0)
		return;
	if (fstat(wtmp_file.fd, &s) < 0)
		goto set_file_error;
	/*
	 * We must have a file at least 2 sizeof(struct utmpx) in size,
	 * with the first struct utmpx matching a signature record.
	 */
#ifdef __LP64__
	if ((wtmp_file.count = s.st_size / sizeof(struct utmpx32)) <= 1)
#else /* __LP64__ */
	if ((wtmp_file.count = s.st_size / sizeof(struct utmpx)) <= 1)
#endif /* __LP64__ */
		goto set_file_error;
#ifdef __LP64__
	if (read(wtmp_file.fd, wtmp_file.buf, sizeof(struct utmpx32)) != sizeof(struct utmpx32))
#else /* __LP64__ */
	if (read(wtmp_file.fd, wtmp_file.buf, sizeof(struct utmpx)) != sizeof(struct utmpx))
#endif /* __LP64__ */
		goto set_file_error;
	if (strcmp(wtmp_file.buf->ut_user, _utmpx_vers) != 0 ||
	    wtmp_file.buf->ut_type != SIGNATURE)
		goto set_file_error;
	wtmp_file.count--;

	/*
	 * We will first read any records modulo WTMP_COUNT (or WTMP_COUNT),
	 * either at the beginning or the end, so that all subsequent reads
	 * must contain WTMP_COUNT records.
	 */
	c = WTMP_COUNT * ((wtmp_file.count - 1) / WTMP_COUNT);
	wtmp_file.left = wtmp_file.count - c;
	wtmp_file.count -= wtmp_file.left;

	/* Seek to the end for reverse reading */
	if (wtmp_file.dir < 0) {
#ifdef __LP64__
		wtmp_file.off = (c + 1) * sizeof(struct utmpx32);
#else /* __LP64__ */
		wtmp_file.off = (c + 1) * sizeof(struct utmpx);
#endif /* __LP64__ */
		if (lseek(wtmp_file.fd, wtmp_file.off, SEEK_SET) < 0)
			goto set_file_error;
	}
#ifdef __LP64__
	n = wtmp_file.left * sizeof(struct utmpx32);
#else /* __LP64__ */
	n = wtmp_file.left * sizeof(struct utmpx);
#endif /* __LP64__ */
	cp = (char *)wtmp_file.buf;
	do {
		if((r = read(wtmp_file.fd, cp, n)) <= 0) {
			if (r < 0 && (errno == EINTR || errno == EAGAIN))
				continue;
			goto set_file_error;
		}
		cp += r;
	} while((n -= r) > 0);

	/* Point to either the beginning or end of the buffer */
	if(wtmp_file.dir > 0)
		wtmp_file.next = wtmp_file.buf;
	else
		wtmp_file.next = wtmp_file.buf + wtmp_file.left - 1;
	return;

set_file_error:
	wtmp_file.left = 0;
	close(wtmp_file.fd);
	wtmp_file.fd = -1;
	return;
}

#ifdef __LP64__
/*
 * these routines assume natural alignment so that struct utmpx32 has
 * the same size and layout as the 32-bit struct utmpx
 */
__private_extern__ void
_utmpx32_64(const struct utmpx32 *u32, struct utmpx *u)
{
	bzero(u, sizeof(*u));
	memcpy(u, u32, offsetof(struct utmpx, ut_type) + sizeof(u->ut_type));
	u->ut_tv.tv_sec = u32->ut_tv.tv_sec;
	u->ut_tv.tv_usec = u32->ut_tv.tv_usec;
	memcpy((char *)u + offsetof(struct utmpx, ut_host),
	    (char *)u32 + offsetof(struct utmpx32, ut_host),
	    sizeof(struct utmpx) - offsetof(struct utmpx, ut_host));
}

__private_extern__ void
_utmpx64_32(const struct utmpx *u, struct utmpx32 *u32)
{
	bzero(u32, sizeof(*u32));
	memcpy(u32, u, offsetof(struct utmpx32, ut_type) + sizeof(u32->ut_type));
	u32->ut_tv.tv_sec = u->ut_tv.tv_sec;
	u32->ut_tv.tv_usec = u->ut_tv.tv_usec;
	memcpy((char *)u32 + offsetof(struct utmpx32, ut_host),
	    (char *)u + offsetof(struct utmpx, ut_host),
	    sizeof(struct utmpx32) - offsetof(struct utmpx32, ut_host));
}
#endif /* __LP64__ */

#ifdef UTMP_COMPAT
#ifdef __LP64__
__private_extern__ void
_getutmp32(const struct utmpx *ux, struct utmp32 *u)
{

	bzero(u, sizeof(*u));
	(void)memcpy(u->ut_name, ux->ut_user, sizeof(u->ut_name));
	(void)memcpy(u->ut_line, ux->ut_line, sizeof(u->ut_line));
	(void)memcpy(u->ut_host, ux->ut_host, sizeof(u->ut_host));
	u->ut_time = ux->ut_tv.tv_sec;
}
#endif /* __LP64__ */

/*
 * _utmp_compat converts a struct utmpx to a struct utmp, using the conventions
 * described in utmp(5).  It then returns a value that specifies what
 * combination of utmp, wtmp and lastlog to write.  UTMP_COMPAT_UTMP1 will
 * write utmp only if a matching record with the same ut_line value is found;
 * UTMP_COMPAT_UTMP0 replaces an existing record or writes a new one.
 */
__private_extern__ int
#ifdef __LP64__
_utmp_compat(const struct utmpx *ux, struct utmp32 *u)
#else /* __LP64__ */
_utmp_compat(const struct utmpx *ux, struct utmp *u)
#endif /* __LP64__ */
{
#ifdef __LP64__
	_getutmp32(ux, u);
#else /* __LP64__ */
	getutmp(ux, u);
#endif /* __LP64__ */

	switch (ux->ut_type) {
	case BOOT_TIME:
	case SHUTDOWN_TIME:
		bzero(u->ut_line, sizeof(u->ut_line));
		u->ut_line[0] = '~';
		bzero(u->ut_name, sizeof(u->ut_name));
		strcpy(u->ut_name, (ux->ut_type == BOOT_TIME ? "reboot" : "shutdown"));
		return UTMP_COMPAT_WTMP;
	case OLD_TIME:
	case NEW_TIME:
		bzero(u->ut_line, sizeof(u->ut_line));
		u->ut_line[0] = (ux->ut_type == OLD_TIME ? '|' : '{');
		bzero(u->ut_name, sizeof(u->ut_name));
		strcpy(u->ut_name, "date");
		return UTMP_COMPAT_WTMP;
	case USER_PROCESS:
		return UTMP_COMPAT_UTMP0 | UTMP_COMPAT_WTMP | UTMP_COMPAT_LASTLOG;
	case DEAD_PROCESS:
		bzero(u->ut_name, sizeof(u->ut_name));
		bzero(u->ut_host, sizeof(u->ut_host));
		return UTMP_COMPAT_UTMP1 | UTMP_COMPAT_WTMP;
	}
	return 0;	/* skip */;
}

/*
 * Write _PATH_LASTLOG given a struct utmp record.  We use
 * advisory record locking.
 */
__private_extern__ void
#ifdef __LP64__
_write_lastlog(const struct utmp32 *u, const struct utmpx *ux)
#else /* __LP64__ */
_write_lastlog(const struct utmp *u, const struct utmpx *ux)
#endif /* __LP64__ */
{
	int fd;
#ifdef __LP64__
	struct lastlog32 l;
#else /* __LP64__ */
	struct lastlog l;
#endif /* __LP64__ */
	struct flock lock;
	struct passwd pw;
	// sizeof(ux->ut_user) > sizeof(u->ut_name)
	char name[sizeof(ux->ut_user) + 1];
	char *buf;
	off_t off;
	int retry = 10;

	if (ux) {
		if(!*ux->ut_user)
			return;
		strncpy(name, ux->ut_user, sizeof(ux->ut_user));
		name[sizeof(ux->ut_user)] = 0;
	} else {
		if (!*u->ut_name)
			return;
		strncpy(name, u->ut_name, sizeof(u->ut_name));
		name[sizeof(u->ut_name)] = 0;
	}
	if ((buf = _pwnam_r(name, &pw)) == NULL)
		return;
#ifdef __LP64__
	off = (off_t)pw.pw_uid * sizeof(struct lastlog32);
#else /* __LP64__ */
	off = (off_t)pw.pw_uid * sizeof(struct lastlog);
#endif /* __LP64__ */
	free(buf);

	if ((fd = open(_PATH_LASTLOG, O_WRONLY, 0)) < 0)
		return;
	(void)lseek(fd, off, SEEK_SET);
	bzero(&lock, sizeof(lock));
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = off;
#ifdef __LP64__
	lock.l_len = sizeof(struct lastlog32);
#else /* __LP64__ */
	lock.l_len = sizeof(struct lastlog);
#endif /* __LP64__ */
	/* try to lock, but give up after retry times, and write anyways */
	while(retry-- > 0) {
		if (fcntl(fd, F_SETLK, &lock) == 0)
			break;
		usleep(10000);
	}
	l.ll_time = u->ut_time;
	strncpy(l.ll_line, u->ut_line, sizeof(l.ll_line));
	strncpy(l.ll_host, u->ut_host, sizeof(l.ll_host));
	(void) write(fd, &l, sizeof(l));
	lock.l_type = F_UNLCK;
	(void) fcntl(fd, F_SETLK, &lock);
	(void) close(fd);
}

/*
 * Write _PATH_UTMP, given a struct utmp, depending on the value of
 * "mustexist".
 */
__private_extern__ void
#ifdef __LP64__
_write_utmp(const struct utmp32 *u, int mustexist)
#else /* __LP64__ */
_write_utmp(const struct utmp *u, int mustexist)
#endif /* __LP64__ */
{
	int fd, slot;
	struct ttyent *ttyp;
#ifdef __LP64__
	struct utmp32 tmp;
#else /* __LP64__ */
	struct utmp tmp;
#endif /* __LP64__ */
	int found = 0;
	static struct {
		char line[sizeof(u->ut_line)];
		int slot;
	} cache;

	if ((fd = open(_PATH_UTMP, O_RDWR, 0)) < 0)
		return;

	if (!strncmp(cache.line, u->ut_line, sizeof(u->ut_line))) {
		slot = cache.slot;
		found++;
	}
	/* do equivalent of ttyslot(), but using u->ut_line */
	if (!found) {
		setttyent();
		slot = 1;
		for(;;) {
			if ((ttyp = getttyent()) == NULL)
				break;
			if (!strncmp(ttyp->ty_name, u->ut_line, sizeof(u->ut_line))) {
				strncpy(cache.line, u->ut_line, sizeof(u->ut_line));
				cache.slot = slot;
				found++;
				break;
			}
			slot++;
		}
		endttyent();
	}

	if (!found) {	/* no assigned slot */
#ifdef __LP64__
		(void)lseek(fd, (off_t)slot * sizeof(struct utmp32), SEEK_SET);
#else /* __LP64__ */
		(void)lseek(fd, (off_t)slot * sizeof(struct utmp), SEEK_SET);
#endif /* __LP64__ */
		for(;;) {
			if (read(fd, &tmp, sizeof(tmp)) != sizeof(tmp))
				break;
			if (!strncmp(tmp.ut_line, u->ut_line, sizeof(u->ut_line))) {
				strncpy(cache.line, u->ut_line, sizeof(u->ut_line));
				cache.slot = slot;
				found++;
				break;
			}
			slot++;
		}
	}

	if (!found && mustexist) {
		(void)close(fd);
		return;
	}
#ifdef __LP64__
	(void)lseek(fd, (off_t)slot * sizeof(struct utmp32), SEEK_SET);
	(void)write(fd, u, sizeof(struct utmp32));
#else /* __LP64__ */
	(void)lseek(fd, (off_t)slot * sizeof(struct utmp), SEEK_SET);
	(void)write(fd, u, sizeof(struct utmp));
#endif /* __LP64__ */
	(void)close(fd);
}

/*
 * Write all the necessary files (utmp, wtmp, lastlog), depending on the
 * given struct utmpx.
 */
__private_extern__ void
_write_utmp_compat(const struct utmpx *ux)
{
#ifdef __LP64__
	struct utmp32 u;
#else /* __LP64__ */
	struct utmp u;
#endif /* __LP64__ */
	int which;

	which = _utmp_compat(ux, &u);
	if (which & UTMP_COMPAT_UTMP0)
		_write_utmp(&u, 0);
	else if (which & UTMP_COMPAT_UTMP1)
		_write_utmp(&u, 1);
	if (which & UTMP_COMPAT_WTMP)
		_write_wtmp(&u);
	if (which & UTMP_COMPAT_LASTLOG)
		_write_lastlog(&u, ux);
}

/* Append a struct wtmp to _PATH_WTMP */
__private_extern__ void
#ifdef __LP64__
_write_wtmp(const struct utmp32 *u)
#else /* __LP64__ */
_write_wtmp(const struct utmp *u)
#endif /* __LP64__ */
{
	int fd;
	struct stat buf;

	if ((fd = open(_PATH_WTMP, O_WRONLY | O_APPEND, 0)) < 0)
		return;
	if (fstat(fd, &buf) == 0) {
		if (write(fd, u, sizeof(*u)) != sizeof(*u))
			(void) ftruncate(fd, buf.st_size);
	}
	(void) close(fd);
}
#endif /* UTMP_COMPAT */

/*
 * thread aware SPI
 */
utmpx_t
_openutx(const char *name)
{
	struct _utmpx *U;

	if ((U = calloc(1, sizeof(struct _utmpx))) == NULL)
		return NULL;
	memcpy(U->magic, __utx_magic__, UTMPX_MAGIC);
	U->utmpx_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	if (__utmpxname(U, name) == 0) {
		if (!U->utfile_system)
			free(U->utfile);
		free(U);
		errno = EINVAL;
		return NULL;
	}
	return (utmpx_t)U;
}

int
_closeutx(utmpx_t u)
{
	struct _utmpx *U = (struct _utmpx *)u;

	if (!U || memcmp(U->magic, __utx_magic__, UTMPX_MAGIC) != 0) {
		errno = EINVAL;
		return -1;
	}
	UTMPX_LOCK(U);
	__endutxent(U);
	if (!U->utfile_system)
		free(U->utfile);
	UTMPX_UNLOCK(U);
	free(U);
	return 0;
}
