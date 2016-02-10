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
 * Copyright (c) 1996 Theo de Raadt <deraadt@theos.com>
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
static char *rcsid = "$OpenBSD: yp_bind.c,v 1.9 1997/04/29 21:25:20 deraadt Exp $";
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
#include <notify.h>
#include <pthread.h>
#include "ypinternal.h"

extern int notify_register_plain(char *, int *);

struct dom_binding *_ypbindlist = NULL;
char _yp_domain[MAXHOSTNAMELEN] = { '\0' };

int _yplib_timeout = 10;

int
_yp_dobind(dom, ypdb)
	const char     *dom;
	struct dom_binding **ypdb;
{
	static int      pid = -1;
	struct dom_binding *ysd, *ysd2;
	struct ypbind_resp ypbr;
	struct timeval  tv;
	struct sockaddr_in clnt_sin;
	struct ypbind_binding *bn;
	int             clnt_sock, fd, gpid;
	CLIENT         *client;
	int             new = 0, r, proto;
	int             count = 0;
	u_short		port;
	int status, notify_token;
	uint64_t abort;
	char *notify_name;

	/*
	 * test if YP is running or not
	 */
	if ((fd = open(YPBINDLOCK, O_RDONLY)) == -1) return YPERR_YPBIND;

	if (!((flock(fd, LOCK_EX | LOCK_NB) == -1) && (errno == EWOULDBLOCK)))
	{
		close(fd);
		return YPERR_YPBIND;
	}

	close(fd);

	gpid = getpid();
	if (!((pid == -1) || (pid == gpid)))
	{
		ysd = _ypbindlist;
		while (ysd)
		{
			if (ysd->dom_client) clnt_destroy(ysd->dom_client);
			ysd2 = ysd->dom_pnext;
			free(ysd);
			ysd = ysd2;
		}

		_ypbindlist = NULL;
	}

	pid = gpid;

	if (ypdb != NULL) *ypdb = NULL;

	if ((dom == NULL) || (strlen(dom) == 0)) return YPERR_BADARGS;

	for (ysd = _ypbindlist; ysd; ysd = ysd->dom_pnext)
	{
		if (strcmp(dom, ysd->dom_domain) == 0) break;
	}

	if (ysd == NULL)
	{
		ysd = calloc(1, sizeof(struct dom_binding));
		if (ysd == NULL) return YPERR_YPERR;
		ysd->dom_socket = -1;
		ysd->dom_vers = 0;
		new = 1;
	}

	/*
	 * Get notification token
	 * we use a self-notification token to allow a caller
	 * to signal the thread doing this bind call to quit.
	 */
	notify_name = NULL;
	notify_token = -1;

	asprintf(&notify_name, "self.thread.%lu", (unsigned long)pthread_self());
	if (notify_name != NULL) 
	{
		status = notify_register_plain(notify_name, &notify_token);
		free(notify_name);
	}

again:
	if (notify_token != -1)
	{
		abort = 0;
		status = notify_get_state(notify_token, &abort);
		if (abort == ThreadStateExitRequested)
		{
			if (new) free(ysd);
			notify_cancel(notify_token);
			return YPERR_YPBIND;
		}
	}

	proto = YP_BIND_UDP;
	if (ysd->dom_vers == YP_BIND_TCP) proto = YP_BIND_TCP;

	if ((ysd->dom_vers == 0) || (ysd->dom_vers == YP_BIND_UDP) || (ysd->dom_vers == YP_BIND_TCP))
	{
		memset(&clnt_sin, 0, sizeof clnt_sin);
		clnt_sin.sin_len = sizeof(struct sockaddr_in);
		clnt_sin.sin_family = AF_INET;
		clnt_sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

		clnt_sock = RPC_ANYSOCK;
		client = clnttcp_create(&clnt_sin, YPBINDPROG, YPBINDVERS, &clnt_sock, 0, 0);
		if (client == NULL)
		{
			clnt_pcreateerror("clnttcp_create");
			if (new) free(ysd);
			if (notify_token != -1) notify_cancel(notify_token);
			return YPERR_YPBIND;
		}

		if ((ntohs(clnt_sin.sin_port) >= IPPORT_RESERVED) || (ntohs(clnt_sin.sin_port) == 20))
		{
			/*
			 * YP was not running, but someone has registered
			 * ypbind with portmap -- this simply means YP is
			 * not running.
			 */
			clnt_destroy(client);
			if (new) free(ysd);
			if (notify_token != -1) notify_cancel(notify_token);
			return YPERR_YPBIND;
		}

		tv.tv_sec = _yplib_timeout;
		tv.tv_usec = 0;

		r = clnt_call(client, YPBINDPROC_DOMAIN, (xdrproc_t)xdr_domainname, &dom, (xdrproc_t)xdr_ypbind_resp, &ypbr, tv);
		if (r != RPC_SUCCESS)
		{
			if (new == 0 || count) fprintf(stderr, "YP server for domain %s not responding, still trying\n", dom);
			count++;
			clnt_destroy(client);
			ysd->dom_vers = proto;
			goto again;
		}

		clnt_destroy(client);

		bn = &ypbr.ypbind_resp_u.ypbind_bindinfo;
		memcpy(&port, &bn->ypbind_binding_port, sizeof port);
		if ((ntohs(port) >= IPPORT_RESERVED) || (ntohs(port) == 20))
		{
			/*
			 * This is bogus -- the ypbind wants me to
			 * communicate to an insecure ypserv.  We are
			 * within rights to syslog this as an attack,
			 * but for now we'll simply ignore it; real YP
			 * is obviously not running.
			 */
			if (new) free(ysd);
			if (notify_token != -1) notify_cancel(notify_token);
			return YPERR_YPBIND;
		}

		memset(&ysd->dom_server_addr, 0,  sizeof ysd->dom_server_addr);
		ysd->dom_server_addr.sin_len = sizeof(struct sockaddr_in);
		ysd->dom_server_addr.sin_family = AF_INET;
		memcpy(&ysd->dom_server_addr.sin_port, &bn->ypbind_binding_port, sizeof(ysd->dom_server_addr.sin_port));
		memcpy(&ysd->dom_server_addr.sin_addr.s_addr, &bn->ypbind_binding_addr, sizeof(ysd->dom_server_addr.sin_addr.s_addr));
		ysd->dom_server_port = ysd->dom_server_addr.sin_port;
		ysd->dom_vers = YPVERS;
		strncpy(ysd->dom_domain, dom, sizeof ysd->dom_domain-1);
		ysd->dom_domain[sizeof ysd->dom_domain-1] = '\0';
	}

	tv.tv_sec = _yplib_timeout / 2;
	tv.tv_usec = 0;

	if (ysd->dom_client) clnt_destroy(ysd->dom_client);
	ysd->dom_socket = RPC_ANYSOCK;

	if (proto == YP_BIND_UDP) ysd->dom_client = clntudp_create(&ysd->dom_server_addr, YPPROG, YPVERS, tv, &ysd->dom_socket);
	else ysd->dom_client = clnttcp_create(&ysd->dom_server_addr, YPPROG, YPVERS, &ysd->dom_socket, 0, 0);

	if (ysd->dom_client == NULL)
	{
		if (proto == YP_BIND_UDP) clnt_pcreateerror("clntudp_create");
		else clnt_pcreateerror("clnttcp_create");
		ysd->dom_vers = proto;
		goto again;
	}

	if (notify_token != -1) notify_cancel(notify_token);

	if (fcntl(ysd->dom_socket, F_SETFD, 1) == -1)
		perror("fcntl: F_SETFD");

	if (new != 0)
	{
		ysd->dom_pnext = _ypbindlist;
		_ypbindlist = ysd;
	}

	if (ypdb != NULL) *ypdb = ysd;
	return 0;
}

void
_yp_unbind(ypb)
	struct dom_binding *ypb;
{
	if (ypb->dom_client) clnt_destroy(ypb->dom_client);
	ypb->dom_client = NULL;
	ypb->dom_socket = -1;
}

int
yp_bind(dom)
	const char     *dom;
{
	return _yp_dobind(dom, NULL);
}

void
yp_unbind(dom)
	const char     *dom;
{
	struct dom_binding *ypb, *ypbp;

	ypbp = NULL;
	for (ypb = _ypbindlist; ypb; ypb = ypb->dom_pnext) {
		if (strcmp(dom, ypb->dom_domain) == 0) {
			if (ypb->dom_client) clnt_destroy(ypb->dom_client);
			if (ypbp)
				ypbp->dom_pnext = ypb->dom_pnext;
			else
				_ypbindlist = ypb->dom_pnext;
			free(ypb);
			return;
		}
		ypbp = ypb;
	}
}
