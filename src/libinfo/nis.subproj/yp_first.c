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
static char *rcsid = "$OpenBSD: yp_first.c,v 1.5 1996/12/03 08:20:03 deraadt Exp $";
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
#include "ypinternal.h"

int
yp_first(indomain, inmap, outkey, outkeylen, outval, outvallen)
	const char     *indomain;
	const char     *inmap;
	char          **outkey;
	int            *outkeylen;
	char          **outval;
	int            *outvallen;
{
	struct ypresp_key_val yprkv;
	struct ypreq_nokey yprnk;
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

	yprnk.domain = (char *)indomain;
	yprnk.map = (char *)inmap;
	(void)memset(&yprkv, 0, sizeof yprkv);

	r = clnt_call(ysd->dom_client, YPPROC_FIRST, (xdrproc_t)xdr_ypreq_nokey, &yprnk, (xdrproc_t)xdr_ypresp_key_val, &yprkv, tv);
	if (r != RPC_SUCCESS)
	{
		if (tries++) clnt_perror(ysd->dom_client, "yp_first: clnt_call");

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
