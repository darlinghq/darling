/*
 * Copyright (c) 1999 by Internet Software Consortium, Inc.
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

#ifndef __APPLE__
#ifndef lint
static const char rcsid[] = "$Id: ns_sign.c,v 1.1 2006/03/01 19:01:37 majka Exp $";
#endif
#endif

/* Import. */

#ifndef __APPLE__
#include "port_before.h"
#include "fd_setsize.h"
#endif

#include <sys/types.h>
#include <sys/param.h>

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>

#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifndef __APPLE__
#include <isc/dst.h>
#include "port_after.h"
#else
#include "dst_internal.h"
#include "res_private.h"
#endif

#define BOUNDS_CHECK(ptr, count) \
	do { \
		if ((ptr) + (count) > eob) { \
			errno = EMSGSIZE; \
			return(NS_TSIG_ERROR_NO_SPACE); \
		} \
	} while (0)

/* ns_sign
 * Parameters:
 *	msg		message to be sent
 *	msglen		input - length of message
 *			output - length of signed message
 *	msgsize		length of buffer containing message
 *	error		value to put in the error field
 *	key		tsig key used for signing
 *	querysig	(response), the signature in the query
 *	querysiglen	(response), the length of the signature in the query
 *	sig		a buffer to hold the generated signature
 *	siglen		input - length of signature buffer
 *			output - length of signature
 *
 * Errors:
 *	- bad input data (-1)
 *	- bad key / sign failed (-BADKEY)
 *	- not enough space (NS_TSIG_ERROR_NO_SPACE)
 */
int
ns_sign(u_char *msg, int *msglen, int msgsize, int error, void *k,
	const u_char *querysig, int querysiglen, u_char *sig, int *siglen,
	time_t in_timesigned)
{
	return(ns_sign2(msg, msglen, msgsize, error, k,
			querysig, querysiglen, sig, siglen,
			in_timesigned, NULL, NULL));
}

int
ns_sign2(u_char *msg, int *msglen, int msgsize, int error, void *k,
	 const u_char *querysig, int querysiglen, u_char *sig, int *siglen,
	 time_t in_timesigned, u_char **dnptrs, u_char **lastdnptr)
{
	HEADER *hp = (HEADER *)msg;
	DST_KEY *key = (DST_KEY *)k;
	u_char *cp = msg + *msglen, *eob = msg + msgsize;
	u_char *lenp;
	u_char *name, *alg;
	int n;
	time_t timesigned;

	dst_init();
	if (msg == NULL || msglen == NULL || sig == NULL || siglen == NULL)
		return (-1);

	/* Name. */
	if (key != NULL && error != ns_r_badsig && error != ns_r_badkey)
		n = dn_comp(key->dk_key_name, cp, eob - cp, dnptrs, lastdnptr);
	else
		n = dn_comp("", cp, eob - cp, NULL, NULL);
	if (n < 0)
		return (NS_TSIG_ERROR_NO_SPACE);
	name = cp;
	cp += n;

	/* Type, class, ttl, length (not filled in yet). */
	BOUNDS_CHECK(cp, NS_INT16SZ + NS_INT16SZ + NS_INT32SZ + NS_INT16SZ);
	NS_PUT16(ns_t_tsig, cp);
	NS_PUT16(ns_c_any, cp);
	NS_PUT32(0, cp);		/* TTL */
	lenp = cp;
	cp += 2;

	/* Alg. */
	if (key != NULL && error != ns_r_badsig && error != ns_r_badkey) {
		if (key->dk_alg != KEY_HMAC_MD5)
			return (-ns_r_badkey);
		n = dn_comp(NS_TSIG_ALG_HMAC_MD5, cp, eob - cp, NULL, NULL);
	}
	else
		n = dn_comp("", cp, eob - cp, NULL, NULL);
	if (n < 0)
		return (NS_TSIG_ERROR_NO_SPACE);
	alg = cp;
	cp += n;
	
	/* Time. */
	BOUNDS_CHECK(cp, NS_INT16SZ + NS_INT32SZ + NS_INT16SZ);
	NS_PUT16(0, cp);
	timesigned = time(NULL);
	if (error != ns_r_badtime)
		NS_PUT32(timesigned, cp);
	else
		NS_PUT32(in_timesigned, cp);
	NS_PUT16(NS_TSIG_FUDGE, cp);

	/* Compute the signature. */
	if (key != NULL && error != ns_r_badsig && error != ns_r_badkey) {
		void *ctx;
		u_char buf[NS_MAXDNAME], *cp2;
		int n;

		dst_sign_data(SIG_MODE_INIT, key, &ctx, NULL, 0, NULL, 0);

		/* Digest the query signature, if this is a response. */
		if (querysiglen > 0 && querysig != NULL) {
			u_int16_t len_n = htons(querysiglen);
			dst_sign_data(SIG_MODE_UPDATE, key, &ctx,
				      (u_char *)&len_n, NS_INT16SZ, NULL, 0);
			dst_sign_data(SIG_MODE_UPDATE, key, &ctx,
				      querysig, querysiglen, NULL, 0);
		}

		/* Digest the message. */
		dst_sign_data(SIG_MODE_UPDATE, key, &ctx, msg, *msglen,
			      NULL, 0);

		/* Digest the key name. */
		n = ns_name_ntol(name, buf, sizeof(buf));
		dst_sign_data(SIG_MODE_UPDATE, key, &ctx, buf, n, NULL, 0);

		/* Digest the class and TTL. */
		cp2 = buf;
		NS_PUT16(ns_c_any, cp2);
		NS_PUT32(0, cp2);
		dst_sign_data(SIG_MODE_UPDATE, key, &ctx, buf, cp2-buf,
			      NULL, 0);

		/* Digest the algorithm. */
		n = ns_name_ntol(alg, buf, sizeof(buf));
		dst_sign_data(SIG_MODE_UPDATE, key, &ctx, buf, n, NULL, 0);

		/* Digest the time signed, fudge, error, and other data */
		cp2 = buf;
		NS_PUT16(0, cp2);	/* Top 16 bits of time */
		if (error != ns_r_badtime)
			NS_PUT32(timesigned, cp2);
		else
			NS_PUT32(in_timesigned, cp2);
		NS_PUT16(NS_TSIG_FUDGE, cp2);
		NS_PUT16(error, cp2);	/* Error */
		if (error != ns_r_badtime)
			NS_PUT16(0, cp2);	/* Other data length */
		else {
			NS_PUT16(NS_INT16SZ+NS_INT32SZ, cp2);	/* Other data length */
			NS_PUT16(0, cp2);	/* Top 16 bits of time */
			NS_PUT32(timesigned, cp2);
		}
		dst_sign_data(SIG_MODE_UPDATE, key, &ctx, buf, cp2-buf,
			      NULL, 0);

		n = dst_sign_data(SIG_MODE_FINAL, key, &ctx, NULL, 0,
				  sig, *siglen);
		if (n < 0)
			return (-ns_r_badkey);
		*siglen = n;
	} else
		*siglen = 0;

	/* Add the signature. */
	BOUNDS_CHECK(cp, NS_INT16SZ + (*siglen));
	NS_PUT16(*siglen, cp);
	memcpy(cp, sig, *siglen);
	cp += (*siglen);

	/* The original message ID & error. */
	BOUNDS_CHECK(cp, NS_INT16SZ + NS_INT16SZ);
	NS_PUT16(ntohs(hp->id), cp);	/* already in network order */
	NS_PUT16(error, cp);

	/* Other data. */
	BOUNDS_CHECK(cp, NS_INT16SZ);
	if (error != ns_r_badtime)
		NS_PUT16(0, cp);	/* Other data length */
	else {
		NS_PUT16(NS_INT16SZ+NS_INT32SZ, cp);	/* Other data length */
		BOUNDS_CHECK(cp, NS_INT32SZ+NS_INT16SZ);
		NS_PUT16(0, cp);	/* Top 16 bits of time */
		NS_PUT32(timesigned, cp);
	}

	/* Go back and fill in the length. */
	NS_PUT16(cp - lenp - NS_INT16SZ, lenp);

	hp->arcount = htons(ntohs(hp->arcount) + 1);
	*msglen = (cp - msg);
	return (0);
}

int
ns_sign_tcp_init(void *k, const u_char *querysig, int querysiglen,
		 ns_tcp_tsig_state *state)
{
	dst_init();
	if (state == NULL || k == NULL || querysig == NULL || querysiglen < 0)
		return (-1);
	state->counter = -1;
	state->key = k;
	if (state->key->dk_alg != KEY_HMAC_MD5)
		return (-ns_r_badkey);
	if (querysiglen > (int)sizeof(state->sig))
		return (-1);
	memcpy(state->sig, querysig, querysiglen);
	state->siglen = querysiglen;
	return (0);
}

int
ns_sign_tcp(u_char *msg, int *msglen, int msgsize, int error,
	    ns_tcp_tsig_state *state, int done)
{
	return (ns_sign_tcp2(msg, msglen, msgsize, error, state,
			     done, NULL, NULL));
}

int
ns_sign_tcp2(u_char *msg, int *msglen, int msgsize, int error,
	     ns_tcp_tsig_state *state, int done,
	     u_char **dnptrs, u_char **lastdnptr)
{
	u_char *cp, *eob, *lenp;
	u_char buf[NS_MAXDNAME], *cp2;
	HEADER *hp = (HEADER *)msg;
	time_t timesigned;
	int n;

	if (msg == NULL || msglen == NULL || state == NULL)
		return (-1);

	state->counter++;
	if (state->counter == 0)
		return (ns_sign2(msg, msglen, msgsize, error, state->key,
				 state->sig, state->siglen,
				 state->sig, &state->siglen, 0,
				 dnptrs, lastdnptr));

	if (state->siglen > 0) {
		u_int16_t siglen_n = htons(state->siglen);
		dst_sign_data(SIG_MODE_INIT, state->key, &state->ctx,
			      NULL, 0, NULL, 0);
		dst_sign_data(SIG_MODE_UPDATE, state->key, &state->ctx,
			      (u_char *)&siglen_n, NS_INT16SZ, NULL, 0);
		dst_sign_data(SIG_MODE_UPDATE, state->key, &state->ctx,
			      state->sig, state->siglen, NULL, 0);
		state->siglen = 0;
	}

	dst_sign_data(SIG_MODE_UPDATE, state->key, &state->ctx, msg, *msglen,
		      NULL, 0);

	if (done == 0 && (state->counter % 100 != 0))
		return (0);

	cp = msg + *msglen;
	eob = msg + msgsize;

	/* Name. */
	n = dn_comp(state->key->dk_key_name, cp, eob - cp, dnptrs, lastdnptr);
	if (n < 0)
		return (NS_TSIG_ERROR_NO_SPACE);
	cp += n;

	/* Type, class, ttl, length (not filled in yet). */
	BOUNDS_CHECK(cp, NS_INT16SZ + NS_INT16SZ + NS_INT32SZ + NS_INT16SZ);
	NS_PUT16(ns_t_tsig, cp);
	NS_PUT16(ns_c_any, cp);
	NS_PUT32(0, cp);		/* TTL */
	lenp = cp;
	cp += 2;

	/* Alg. */
	n = dn_comp(NS_TSIG_ALG_HMAC_MD5, cp, eob - cp, NULL, NULL);
	if (n < 0)
		return (NS_TSIG_ERROR_NO_SPACE);
	cp += n;
	
	/* Time. */
	BOUNDS_CHECK(cp, NS_INT16SZ + NS_INT32SZ + NS_INT16SZ);
	NS_PUT16(0, cp);
	timesigned = time(NULL);
	NS_PUT32(timesigned, cp);
	NS_PUT16(NS_TSIG_FUDGE, cp);

	/*
	 * Compute the signature.
	 */

	/* Digest the time signed and fudge. */
	cp2 = buf;
	NS_PUT16(0, cp2);	/* Top 16 bits of time */
	NS_PUT32(timesigned, cp2);
	NS_PUT16(NS_TSIG_FUDGE, cp2);

	dst_sign_data(SIG_MODE_UPDATE, state->key, &state->ctx,
		      buf, cp2 - buf, NULL, 0);

	n = dst_sign_data(SIG_MODE_FINAL, state->key, &state->ctx, NULL, 0,
			  state->sig, sizeof(state->sig));
	if (n < 0)
		return (-ns_r_badkey);
	state->siglen = n;

	/* Add the signature. */
	BOUNDS_CHECK(cp, NS_INT16SZ + state->siglen);
	NS_PUT16(state->siglen, cp);
	memcpy(cp, state->sig, state->siglen);
	cp += state->siglen;

	/* The original message ID & error. */
	BOUNDS_CHECK(cp, NS_INT16SZ + NS_INT16SZ);
	NS_PUT16(ntohs(hp->id), cp);	/* already in network order */
	NS_PUT16(error, cp);

	/* Other data. */
	BOUNDS_CHECK(cp, NS_INT16SZ);
	NS_PUT16(0, cp);

	/* Go back and fill in the length. */
	NS_PUT16(cp - lenp - NS_INT16SZ, lenp);

	hp->arcount = htons(ntohs(hp->arcount) + 1);
	*msglen = (cp - msg);
	return (0);
}
