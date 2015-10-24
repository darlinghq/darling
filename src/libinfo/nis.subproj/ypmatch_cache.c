/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Copyright (c) 1992, 1993 Theo de Raadt <deraadt@theos.com>
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Theo de Raadt.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char *rcsid = "$OpenBSD: ypmatch_cache.c,v 1.6 1996/12/03 08:20:06 deraadt Exp $";
#endif /* LIBC_SCCS and not lint */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <rpc/rpc.h>
#include <rpc/xdr.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>
/* #define YPMATCHCACHE */
#include "ypinternal.h"

int _yplib_cache = 5;

#ifdef YPMATCHCACHE
static bool_t
ypmatch_add(map, key, keylen, val, vallen)
	const char     *map;
	const char     *key;
	u_int           keylen;
	char           *val;
	u_int           vallen;
{
	struct ypmatch_ent *ep;
	time_t t;

	(void)time(&t);

	for (ep = ypmc; ep; ep = ep->next)
		if (ep->expire_t < t)
			break;
	if (ep == NULL) {
		if ((ep = malloc(sizeof *ep)) == NULL)
			return 0;
		(void)memset(ep, 0, sizeof *ep);
		if (ypmc)
			ep->next = ypmc;
		ypmc = ep;
	}

	if (ep->key) {
		free(ep->key);
		ep->key = NULL;
	}
	if (ep->val) {
		free(ep->val);
		ep->val = NULL;
	}

	if ((ep->key = malloc(keylen)) == NULL)
		return 0;

	if ((ep->val = malloc(vallen)) == NULL) {
		free(ep->key);
		ep->key = NULL;
		return 0;
	}

	ep->keylen = keylen;
	ep->vallen = vallen;

	(void)memcpy(ep->key, key, ep->keylen);
	(void)memcpy(ep->val, val, ep->vallen);

	if (ep->map) {
		if (strcmp(ep->map, map)) {
			free(ep->map);
			if ((ep->map = strdup(map)) == NULL)
				return 0;
		}
	} else {
		if ((ep->map = strdup(map)) == NULL)
			return 0;
	}

	ep->expire_t = t + _yplib_cache;
	return 1;
}

static bool_t
ypmatch_find(map, key, keylen, val, vallen)
	const char     *map;
	const char     *key;
	u_int           keylen;
	char          **val;
	u_int          *vallen;
{
	struct ypmatch_ent *ep;
	time_t          t;

	if (ypmc == NULL)
		return 0;

	(void) time(&t);

	for (ep = ypmc; ep; ep = ep->next) {
		if (ep->keylen != keylen)
			continue;
		if (strcmp(ep->map, map))
			continue;
		if (memcmp(ep->key, key, keylen))
			continue;
		if (t > ep->expire_t)
			continue;

		*val = ep->val;
		*vallen = ep->vallen;
		return 1;
	}
	return 0;
}
#endif 

int
yp_match(indomain, inmap, inkey, inkeylen, outval, outvallen)
	const char     *indomain;
	const char     *inmap;
	const char     *inkey;
	int             inkeylen;
	char          **outval;
	int            *outvallen;
{
	struct dom_binding *ysd;
	struct ypresp_val yprv;
	struct timeval  tv;
	struct ypreq_key yprk;
	int tries = 0, r;
	static int proto = YP_BIND_UDP;

	if (indomain == NULL || *indomain == '\0' || 
	    strlen(indomain) > YPMAXDOMAIN || inmap == NULL ||
	    *inmap == '\0' || strlen(inmap) > YPMAXMAP ||
	    inkey == NULL || inkeylen == 0)
		return YPERR_BADARGS;

	*outval = NULL;
	*outvallen = 0;

again:
	if (_yp_dobind(indomain, &ysd) != 0)
		return YPERR_DOMAIN;

#ifdef YPMATCHCACHE
	if (!strcmp(_yp_domain, indomain) && ypmatch_find(inmap, inkey,
	    inkeylen, &yprv.val.valdat_val, &yprv.val.valdat_len)) {
		*outvallen = yprv.val.valdat_len;
		if ((*outval = malloc(*outvallen + 1)) == NULL) {
			_yp_unbind(ysd);
			return YPERR_YPERR;
		}
		(void)memcpy(*outval, yprv.val.valdat_val, *outvallen);
		(*outval)[*outvallen] = '\0';
		_yp_unbind(ysd);
		return 0;
	}
#endif

	tv.tv_sec = _yplib_timeout;
	tv.tv_usec = 0;

	yprk.domain = (char *)indomain;
	yprk.map = (char *)inmap;
	yprk.key.keydat_val = (char *) inkey;
	yprk.key.keydat_len = inkeylen;

	memset(&yprv, 0, sizeof yprv);

	r = clnt_call(ysd->dom_client, YPPROC_MATCH, (xdrproc_t)xdr_ypreq_key, &yprk, (xdrproc_t)xdr_ypresp_val, &yprv, tv);
	if (r != RPC_SUCCESS)
	{
		/* call failed - switch protocols and try again */
		if (tries++) clnt_perror(ysd->dom_client, "yp_match: clnt_call");

		if (proto == YP_BIND_UDP) proto = YP_BIND_TCP;
		else proto = YP_BIND_UDP;
		ysd->dom_vers = proto;

		goto again;
	}

	if (!(r = ypprot_err(yprv.stat))) {
		*outvallen = yprv.val.valdat_len;
		if ((*outval = malloc(*outvallen + 1)) == NULL) {
			r = YPERR_YPERR;
			goto out;
		}
		(void)memcpy(*outval, yprv.val.valdat_val, *outvallen);
		(*outval)[*outvallen] = '\0';
#ifdef YPMATCHCACHE
		if (strcmp(_yp_domain, indomain) == 0)
			if (!ypmatch_add(inmap, inkey, inkeylen,
			    *outval, *outvallen))
				r = YPERR_RESRC;
#endif
	}
out:
	xdr_free((xdrproc_t)xdr_ypresp_val, (char *) &yprv);
	_yp_unbind(ysd);
	return r;
}

int
yp_next(indomain, inmap, inkey, inkeylen, outkey, outkeylen, outval, outvallen)
	const char     *indomain;
	const char     *inmap;
	const char     *inkey;
	int             inkeylen;
	char          **outkey;
	int            *outkeylen;
	char          **outval;
	int            *outvallen;
{
	struct ypresp_key_val yprkv;
	struct ypreq_key yprk;
	struct dom_binding *ysd;
	struct timeval  tv;
	int tries = 0, r;
	static int proto = YP_BIND_UDP;

	if (indomain == NULL || *indomain == '\0' ||
	    strlen(indomain) > YPMAXDOMAIN || inmap == NULL ||
	    *inmap == '\0' || strlen(inmap) > YPMAXMAP)
		return YPERR_BADARGS;

	*outkey = *outval = NULL;
	*outkeylen = *outvallen = 0;

again:
	if (_yp_dobind(indomain, &ysd) != 0)
		return YPERR_DOMAIN;

	tv.tv_sec = _yplib_timeout;
	tv.tv_usec = 0;

	yprk.domain = (char *)indomain;
	yprk.map = (char *)inmap;
	yprk.key.keydat_val = (char *)inkey;
	yprk.key.keydat_len = inkeylen;
	(void)memset(&yprkv, 0, sizeof yprkv);

	r = clnt_call(ysd->dom_client, YPPROC_NEXT, (xdrproc_t)xdr_ypreq_key, &yprk, (xdrproc_t)xdr_ypresp_key_val, &yprkv, tv);
	if (r != RPC_SUCCESS)
	{
		/* call failed - switch protocols and try again */
		if (tries++) clnt_perror(ysd->dom_client, "yp_next: clnt_call");

		if (proto == YP_BIND_UDP) proto = YP_BIND_TCP;
		else proto = YP_BIND_UDP;
		ysd->dom_vers = proto;

		goto again;
	}

	if (!(r = ypprot_err(yprkv.stat))) {
		*outkeylen = yprkv.key.keydat_len;
		if ((*outkey = malloc(*outkeylen + 1)) == NULL)
			r = YPERR_RESRC;
		else {
			(void)memcpy(*outkey, yprkv.key.keydat_val, *outkeylen);
			(*outkey)[*outkeylen] = '\0';
		}
		*outvallen = yprkv.val.valdat_len;
		if ((*outval = malloc(*outvallen + 1)) == NULL)
			r = YPERR_RESRC;
		else {
			(void)memcpy(*outval, yprkv.val.valdat_val, *outvallen);
			(*outval)[*outvallen] = '\0';
		}
	}
	xdr_free((xdrproc_t)xdr_ypresp_key_val, (char *) &yprkv);
	_yp_unbind(ysd);
	return r;
}
