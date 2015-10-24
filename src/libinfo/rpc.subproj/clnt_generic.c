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
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 * 
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 * 
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

#if defined(LIBC_SCCS) && !defined(lint)
/*static char *sccsid = "from: @(#)clnt_generic.c 1.4 87/08/11 (C) 1987 SMI";*/
/*static char *sccsid = "from: @(#)clnt_generic.c	2.2 88/08/01 4.0 RPCSRC";*/
static char *rcsid = "$Id: clnt_generic.c,v 1.3 2002/02/19 20:36:22 epeyton Exp $";
#endif

/*
 * Copyright (C) 1987, Sun Microsystems, Inc.
 */
#include <string.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netdb.h>

/*
 * Generic client creation: takes (hostname, program-number, protocol) and
 * returns client handle. Default options are set, which the user can 
 * change using the rpc equivalent of ioctl()'s.
 */
CLIENT *
clnt_create(hostname, prog, vers, proto)
#ifdef __LP64__
	char *hostname;
	uint32_t prog;
	uint32_t vers;
	char *proto;
#else
	char *hostname;
	u_long prog;
	u_long vers;
	char *proto;
#endif
{
	struct hostent *h;
	struct protoent *p;
	struct sockaddr_in sin;
	int sock;
	struct timeval tv;
	CLIENT *client;

	h = gethostbyname(hostname);
	if (h == NULL) {
		rpc_createerr.cf_stat = RPC_UNKNOWNHOST;
		return (NULL);
	}
	if (h->h_addrtype != AF_INET) {
		/*
		 * Only support INET for now
		 */
		rpc_createerr.cf_stat = RPC_SYSTEMERROR;
		rpc_createerr.cf_error.re_errno = EAFNOSUPPORT; 
		return (NULL);
	}
	bzero((char *)&sin, sizeof sin);
	sin.sin_family = h->h_addrtype;
	sin.sin_port = 0;
	bcopy(h->h_addr, (char*)&sin.sin_addr, h->h_length);
	p = getprotobyname(proto);
	if (p == NULL) {
		rpc_createerr.cf_stat = RPC_UNKNOWNPROTO;
		rpc_createerr.cf_error.re_errno = EPFNOSUPPORT; 
		return (NULL);
	}
	sock = RPC_ANYSOCK;
	switch (p->p_proto) {
	case IPPROTO_UDP:
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		client = clntudp_create(&sin, prog, vers, tv, &sock);
		if (client == NULL) {
			return (NULL);
		}
		break;
	case IPPROTO_TCP:
		client = clnttcp_create(&sin, prog, vers, &sock, 0, 0);
		if (client == NULL) {
			return (NULL);
		}
		break;
	default:
		rpc_createerr.cf_stat = RPC_SYSTEMERROR;
		rpc_createerr.cf_error.re_errno = EPFNOSUPPORT; 
		return (NULL);
	}
	return (client);
}
