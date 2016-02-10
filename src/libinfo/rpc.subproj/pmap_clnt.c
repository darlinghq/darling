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
/*static char *sccsid = "from: @(#)pmap_clnt.c 1.37 87/08/11 Copyr 1984 Sun Micro";*/
/*static char *sccsid = "from: @(#)pmap_clnt.c	2.2 88/08/01 4.0 RPCSRC";*/
static char *rcsid = "$Id: pmap_clnt.c,v 1.5 2004/12/19 22:45:44 zarzycki Exp $";
#endif

/*
 * pmap_clnt.c
 * Client interface to pmap rpc service.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#include <string.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "pmap_wakeup.h"

extern CLIENT *clntudp_bufcreate_timeout(struct sockaddr_in *raddr, uint32_t program, uint32_t version, int *sockp, uint32_t sendsz, uint32_t recvsz, struct timeval *timeout, struct timeval *totaltimeout);

static struct timeval set_retry_timeout = { 5, 0 };
static struct timeval set_total_timeout = { 60, 0 };

static struct timeval unset_retry_timeout = { 1, 0 };
static struct timeval unset_total_timeout = { 5, 0 };

void clnt_perror();

/*
 * Set a mapping between program,version and port.
 * Calls the pmap service remotely to do the mapping.
 */
bool_t
pmap_set(program, version, protocol, port)
#ifdef __LP64__
	uint32_t program;
	uint32_t version;
	int32_t protocol;
	uint16_t port;
#else
	u_long program;
	u_long version;
	int protocol;
	u_short port;
#endif
{
	struct sockaddr_in myaddress;
	int socket = -1;
	register CLIENT *client;
	struct pmap parms;
	bool_t rslt;

	if (pmap_wakeup() != 0) return FALSE;

	memset(&myaddress, 0, sizeof(struct sockaddr_in));
	myaddress.sin_family = AF_INET;
	myaddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	client = clntudp_bufcreate_timeout(&myaddress, PMAPPROG, PMAPVERS, &socket, RPCSMALLMSGSIZE, RPCSMALLMSGSIZE, &set_retry_timeout, &set_total_timeout);
	if (client == NULL) return FALSE;

	parms.pm_prog = program;
	parms.pm_vers = version;
	parms.pm_prot = protocol;
	parms.pm_port = port;
	if (CLNT_CALL(client, PMAPPROC_SET, (xdrproc_t)xdr_pmap, &parms, (xdrproc_t)xdr_bool, &rslt, set_total_timeout) != RPC_SUCCESS) return FALSE;

	CLNT_DESTROY(client);
	close(socket);
	return rslt;
}

/*
 * Remove the mapping between program,version and port.
 * Calls the pmap service remotely to do the un-mapping.
 */
bool_t
pmap_unset(program, version)
#ifdef __LP64__
	uint32_t program;
	uint32_t version;
#else
	u_long program;
	u_long version;
#endif
{
	struct sockaddr_in myaddress;
	int socket = -1;
	register CLIENT *client;
	struct pmap parms;
	bool_t rslt;

	if (pmap_wakeup() != 0) return FALSE;

	memset(&myaddress, 0, sizeof(struct sockaddr_in));
	myaddress.sin_family = AF_INET;
	myaddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	client = clntudp_bufcreate_timeout(&myaddress, PMAPPROG, PMAPVERS, &socket, RPCSMALLMSGSIZE, RPCSMALLMSGSIZE, &unset_retry_timeout, &unset_total_timeout);
	if (client == NULL) return FALSE;

	parms.pm_prog = program;
	parms.pm_vers = version;
	parms.pm_port = 0;
	parms.pm_prot = 0;

	CLNT_CALL(client, PMAPPROC_UNSET, (xdrproc_t)xdr_pmap, &parms, (xdrproc_t)xdr_bool, &rslt, unset_total_timeout);

	CLNT_DESTROY(client);
	close(socket);
	return rslt;
}
