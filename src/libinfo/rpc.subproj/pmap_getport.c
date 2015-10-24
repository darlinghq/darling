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
/*static char *sccsid = "from: @(#)pmap_getport.c 1.9 87/08/11 Copyr 1984 Sun Micro";*/
/*static char *sccsid = "from: @(#)pmap_getport.c	2.2 88/08/01 4.0 RPCSRC";*/
static char *rcsid = "$Id: pmap_getport.c,v 1.4 2004/12/19 22:45:44 zarzycki Exp $";
#endif

/*
 * pmap_getport.c
 * Client interface to pmap rpc service.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#include <unistd.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>
#include <sys/socket.h>
#include <net/if.h>

static struct timeval default_timeout = { 5, 0 };

/*
 * Find the mapped port for program,version.
 * Calls the pmap service remotely to do the lookup.
 * Returns 0 if no map exists.
 */
u_short
pmap_getport_timeout(struct sockaddr_in *address, uint32_t program, uint32_t version, uint32_t protocol, struct timeval *timeout, struct timeval *totaltimeout)
{
	u_short port;
	int socket;
	register CLIENT *client;
	struct pmap parms;
	enum clnt_stat status;
	struct timeval real_t, real_tt;

	real_t = default_timeout;
	if (timeout != NULL) real_t = *timeout;

	real_tt = default_timeout;
	if (totaltimeout != NULL) real_tt = *totaltimeout;

	port = 0;
	socket = -1;
	address->sin_port = htons(PMAPPORT);
	client = clntudp_bufcreate(address, PMAPPROG, PMAPVERS, real_t, &socket, RPCSMALLMSGSIZE, RPCSMALLMSGSIZE);
	if (client != NULL)
	{
		parms.pm_prog = program;
		parms.pm_vers = version;
		parms.pm_prot = protocol;
		/* not needed or used */
		parms.pm_port = 0;

		status = CLNT_CALL(client, PMAPPROC_GETPORT, (xdrproc_t)xdr_pmap, &parms, (xdrproc_t)xdr_u_short, &port, real_tt);
		if (status != RPC_SUCCESS)
		{
			rpc_createerr.cf_stat = RPC_PMAPFAILURE;
			clnt_geterr(client, &rpc_createerr.cf_error);
		}
		else if (port == 0)
		{
			rpc_createerr.cf_stat = RPC_PROGNOTREGISTERED;
		}

		CLNT_DESTROY(client);
	}

	close(socket);
	address->sin_port = 0;
	return port;
}

u_short
pmap_getport(address, program, version, protocol)
#ifdef __LP64__
	struct sockaddr_in *address;
	uint32_t program;
	uint32_t version;
	uint32_t protocol;
#else
	struct sockaddr_in *address;
	u_long program;
	u_long version;
	u_int protocol;
#endif
{
	return pmap_getport_timeout(address, (uint32_t)program, (uint32_t)version, (uint32_t)protocol, NULL, NULL);
}

