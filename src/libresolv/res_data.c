/*
 * Copyright (c) 1995-1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$Id: res_data.c,v 1.1 2006/03/01 19:01:37 majka Exp $";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>

#include <ctype.h>
#include <netdb.h>
#include <resolv.h>
#include <res_update.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "res_private.h"

static struct __res_state *_res_static;

#ifdef USE__RES_9
struct __res_9_state _res_9;
#endif

extern int __res_vinit(res_state, int);

const char *__res_opcodes[] = {
	"QUERY",
	"IQUERY",
	"CQUERYM",
	"CQUERYU",	/* experimental */
	"NOTIFY",	/* experimental */
	"UPDATE",
	"6",
	"7",
	"8",
	"9",
	"10",
	"11",
	"12",
	"13",
	"ZONEINIT",
	"ZONEREF",
};

void
__h_errno_set(struct __res_state *res, int err)
{
	h_errno = res->res_h_errno = err;
}

void
res_client_close(res_state res)
{
	if (res == NULL) return;

	if (res->_u._ext.ext != NULL) free(res->_u._ext.ext);
	free(res);
}

res_state
res_state_new()
{
	res_state x;

	x = (res_state)calloc(1, sizeof(struct __res_state));
	if (x == NULL) return NULL;
	
	/*
	 * We use _pad (normally unused) to hold a version number.
	 * We use it provide limited compatibility between versions.
	 */
	x->_pad = 9;

	x->_u._ext.ext = (struct __res_state_ext *)calloc(1, sizeof(struct __res_state_ext));
	if (x->_u._ext.ext == NULL)
	{
		free(x);
		return NULL;
	}

	return x;
}

int
res_init(void)
{
	extern int __res_vinit(res_state, int);
	unsigned int save_retrans, save_retry, save_options, save_id;
	struct __res_state_ext *save_ext;

#ifdef USE__RES_9
	_res_static = &_res_9;
#else
	_res_static = &_res;
#endif

	save_retrans = RES_TIMEOUT;
	save_retry = RES_DFLRETRY;
	save_options = RES_DEFAULT;
	save_id = res_randomid();
	save_ext = _res_static->_u._ext.ext;

	if (_res_static->options & RES_INIT)
	{
		/* Caller wants to override default options */
		save_options = _res_static->options;
		if (_res_static->retrans != 0) save_retrans = _res_static->retrans;
		if (_res_static->retry != 0) save_retry = _res_static->retry;
		if (_res_static->id != 0) save_id = _res_static->id;
	}

	memset(_res_static, 0, sizeof(struct __res_state));
	_res_static->_vcsock = -1;

	_res_static->retrans = save_retrans;
	_res_static->retry = save_retry;
	_res_static->id = save_id;
	_res_static->options = save_options;
	_res_static->_u._ext.ext = save_ext;

	_res_static->_pad = 9;
	
	if (_res_static->_u._ext.ext == NULL) _res_static->_u._ext.ext = (struct __res_state_ext *)calloc(1, sizeof(struct __res_state_ext));

	return (__res_vinit(_res_static, 1));
}

int
res_query(const char *name, int class, int type, u_char *answer, int anslen)	
{
#ifdef USE__RES_9
	_res_static = &_res_9;
#else
	_res_static = &_res;
#endif

	if (((_res_static->options & RES_INIT) == 0) && (res_init() == -1))
	{
		RES_SET_H_ERRNO(_res_static, NETDB_INTERNAL);
		return -1;
	}
	return (res_nquery(_res_static, name, class, type, answer, anslen));
}

void
fp_nquery(const u_char *msg, int len, FILE *file)
{
#ifdef USE__RES_9
	_res_static = &_res_9;
#else
	_res_static = &_res;
#endif
	
	if (((_res_static->options & RES_INIT) == 0) && (res_init() == -1)) return;

	res_pquery(_res_static, msg, len, file);
}

void
fp_query(const u_char *msg, FILE *file)
{
	fp_nquery(msg, NS_PACKETSZ, file);
}

void
p_query(const u_char *msg)
{
	fp_query(msg, stdout);
}

const char *
hostalias(const char *name)
{
	static char abuf[NS_MAXDNAME];

#ifdef USE__RES_9
	_res_static = &_res_9;
#else
	_res_static = &_res;
#endif
	
	return (res_hostalias(_res_static, name, abuf, sizeof abuf));
}

void
res_close(void)
{
#ifdef USE__RES_9
	_res_static = &_res_9;
#else
	_res_static = &_res;
#endif
	
	res_nclose(_res_static);
}

int
res_isourserver(const struct sockaddr_in *inp)
{
#ifdef USE__RES_9
	_res_static = &_res_9;
#else
	_res_static = &_res;
#endif
	
	return (res_ourserver_p(_res_static, (const struct sockaddr *)inp));
}

int
res_nisourserver(const res_state res, const struct sockaddr_in *inp)
{
	return (res_ourserver_p(res, (const struct sockaddr *)inp));
}

int
res_mkquery(int op, const char *dname, int class, int type, const u_char *data, int datalen, const u_char *newrr_in, u_char *buf, int buflen)
{
#ifdef USE__RES_9
	_res_static = &_res_9;
#else
	_res_static = &_res;
#endif
	
	if (((_res_static->options & RES_INIT) == 0) && (res_init() == -1))
	{
		RES_SET_H_ERRNO(_res_static, NETDB_INTERNAL);
		return -1;
	}

	return res_nmkquery(_res_static, op, dname, class, type, data, datalen, newrr_in, buf, buflen);
}

int
res_querydomain(const char *name, const char *domain, int class, int type, u_char *answer, int anslen)
{
#ifdef USE__RES_9
	_res_static = &_res_9;
#else
	_res_static = &_res;
#endif
	
	if (((_res_static->options & RES_INIT) == 0) && (res_init() == -1))
	{
		RES_SET_H_ERRNO(_res_static, NETDB_INTERNAL);
		return -1;
	}

	return res_nquerydomain(_res_static, name, domain, class, type, answer, anslen);
}

int
res_search(const char *name, int class, int type, u_char *answer, int anslen)
{
#ifdef USE__RES_9
	_res_static = &_res_9;
#else
	_res_static = &_res;
#endif
	
	if (((_res_static->options & RES_INIT) == 0) && (res_init() == -1))
	{
		RES_SET_H_ERRNO(_res_static, NETDB_INTERNAL);
		return -1;
	}

	return res_nsearch(_res_static, name, class, type, answer, anslen);
}

int
res_send(const u_char *buf, int buflen, u_char *ans, int anssiz)
{
#ifdef USE__RES_9
	_res_static = &_res_9;
#else
	_res_static = &_res;
#endif
	
	if (((_res_static->options & RES_INIT) == 0) && (res_init() == -1))
	{
		/* errno should have been set by res_init() in this case. */
		return -1;
	}

	return res_nsend(_res_static, buf, buflen, ans, anssiz);
}

int
res_sendsigned(const u_char *buf, int buflen, ns_tsig_key *key, u_char *ans, int anssiz)
{
#ifdef USE__RES_9
	_res_static = &_res_9;
#else
	_res_static = &_res;
#endif
	
	if (((_res_static->options & RES_INIT) == 0) && (res_init() == -1))
	{
		/* errno should have been set by res_init() in this case. */
		return -1;
	}

	return res_nsendsigned(_res_static, buf, buflen, key, ans, anssiz);
}
