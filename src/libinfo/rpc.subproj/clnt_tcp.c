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
/*static char *sccsid = "from: @(#)clnt_tcp.c 1.37 87/10/05 Copyr 1984 Sun Micro";*/
/*static char *sccsid = "from: @(#)clnt_tcp.c	2.2 88/08/01 4.0 RPCSRC";*/
static char *rcsid = "$Id: clnt_tcp.c,v 1.4 2002/03/15 22:07:48 majka Exp $";
#endif
 
/*
 * clnt_tcp.c, Implements a TCP/IP based, client side RPC.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * TCP based RPC supports 'batched calls'.
 * A sequence of calls may be batched-up in a send buffer.  The rpc call
 * returns immediately to the client even though the call was not necessarily
 * sent.  The batching occurs if the results' xdr routine is NULL (0) AND
 * the rpc timeout value is zero (see clnt.h, rpc).
 *
 * Clients should NOT casually batch calls that in fact return results; that is,
 * the server side should be aware that a call is batched and not produce any
 * return message.  Batched calls that produce many result messages can
 * deadlock (netlock) the client and the server....
 *
 * Now go hang yourself.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <rpc/pmap_clnt.h>

#define MCALL_MSG_SIZE 24

extern int errno;

extern int	bindresvport();
extern bool_t	xdr_opaque_auth();

extern u_short pmap_getport_timeout(struct sockaddr_in *address, uint32_t program, uint32_t version, uint32_t protocol, struct timeval *timeout, struct timeval *totaltimeout);

static int	readtcp();
static int	writetcp();

static enum clnt_stat	clnttcp_call();
static void		clnttcp_abort();
static void		clnttcp_geterr();
static bool_t		clnttcp_freeres();
static bool_t           clnttcp_control();
static void		clnttcp_destroy();

static struct clnt_ops tcp_ops = {
	clnttcp_call,
	clnttcp_abort,
	clnttcp_geterr,
	clnttcp_freeres,
	clnttcp_destroy,
	clnttcp_control
};

struct ct_data {
	int		ct_sock;
	bool_t		ct_closeit;
	struct timeval	ct_timeout;
	bool_t          ct_timeout_set;       /* timeout set by clnt_control? */
	struct sockaddr_in ct_addr; 
	struct rpc_err	ct_error;
	char		ct_mcall[MCALL_MSG_SIZE];	/* marshalled callmsg */
	u_int		ct_mpos;			/* pos after marshal */
	XDR		ct_xdrs;
};

/*
 * Create a client handle for a tcp/ip connection.
 * If *sockp<0, *sockp is set to a newly created TCP socket and it is
 * connected to raddr.  If *sockp non-negative then
 * raddr is ignored.  The rpc/tcp package does buffering
 * similar to stdio, so the client must pick send and receive buffer sizes,];
 * 0 => use the default.
 * If raddr->sin_port is 0, then a binder on the remote machine is
 * consulted for the right port number.
 * NB: *sockp is copied into a private area.
 * NB: It is the clients responsibility to close *sockp.
 * NB: The rpch->cl_auth is set null authentication.  Caller may wish to set this
 * something more useful.
 */
CLIENT *
clnttcp_create_timeout(struct sockaddr_in *raddr, uint32_t prog, uint32_t vers, int *sockp, uint32_t sendsz, uint32_t recvsz, struct timeval *retry_timeout, struct timeval *total_timeout)
{
	CLIENT *h;
	register struct ct_data *ct = NULL;
	struct timeval now;
	struct rpc_msg call_msg;
	int rfd;
	u_short port;

	h = (CLIENT *)mem_alloc(sizeof(*h));
	if (h == NULL)
	{
		rpc_createerr.cf_stat = RPC_SYSTEMERROR;
		rpc_createerr.cf_error.re_errno = errno;
		goto fooy;
	}

	ct = (struct ct_data *)mem_alloc(sizeof(*ct));
	if (ct == NULL)
	{
		rpc_createerr.cf_stat = RPC_SYSTEMERROR;
		rpc_createerr.cf_error.re_errno = errno;
		goto fooy;
	}

	/*
	 * If no port number given ask the pmap for one
	 */
	if (raddr->sin_port == 0)
	{
		port = pmap_getport_timeout(raddr, prog, vers, IPPROTO_TCP, retry_timeout, total_timeout);
		if (port == 0)
		{
			mem_free((caddr_t)ct, sizeof(struct ct_data));
			mem_free((caddr_t)h, sizeof(CLIENT));
			return NULL;
		}

		raddr->sin_port = htons(port);
	}

	/*
	 * If no socket given, open one
	 */
	if (*sockp < 0)
	{
		*sockp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		bindresvport(*sockp, (struct sockaddr_in *)0);
		if ((*sockp < 0) || (connect(*sockp, (struct sockaddr *)raddr, sizeof(*raddr)) < 0))
		{
			rpc_createerr.cf_stat = RPC_SYSTEMERROR;
			rpc_createerr.cf_error.re_errno = errno;
			close(*sockp);
			goto fooy;
		}

		ct->ct_closeit = TRUE;
	}
	else
	{
		ct->ct_closeit = FALSE;
	}

	/*
	 * Set up private data struct
	 */
	ct->ct_sock = *sockp;
	ct->ct_timeout.tv_sec = 60;
	ct->ct_timeout.tv_usec = 0;
	ct->ct_timeout_set = FALSE;
	if (total_timeout != NULL)
	{
		ct->ct_timeout = *total_timeout;
		ct->ct_timeout_set = TRUE;
	}
	ct->ct_addr = *raddr;

	/*
	 * Initialize call message
	 */
	rfd = open("/dev/random", O_RDONLY, 0);
	if ((rfd < 0) || (read(rfd, &call_msg.rm_xid, sizeof(call_msg.rm_xid)) != sizeof(call_msg.rm_xid)))
	{
		gettimeofday(&now, (struct timezone *)0);
		call_msg.rm_xid = getpid() ^ now.tv_sec ^ now.tv_usec;
	}

	if (rfd > 0) close(rfd);

	call_msg.rm_direction = CALL;
	call_msg.rm_call.cb_rpcvers = RPC_MSG_VERSION;
	call_msg.rm_call.cb_prog = prog;
	call_msg.rm_call.cb_vers = vers;

	/*
	 * pre-serialize the staic part of the call msg and stash it away
	 */
	xdrmem_create(&(ct->ct_xdrs), ct->ct_mcall, MCALL_MSG_SIZE, XDR_ENCODE);
	if (! xdr_callhdr(&(ct->ct_xdrs), &call_msg))
	{
		if (ct->ct_closeit) close(*sockp);
		goto fooy;
	}

	ct->ct_mpos = XDR_GETPOS(&(ct->ct_xdrs));
	XDR_DESTROY(&(ct->ct_xdrs));

	/*
	 * Create a client handle which uses xdrrec for serialization
	 * and authnone for authentication.
	 */
	xdrrec_create(&(ct->ct_xdrs), sendsz, recvsz, (caddr_t)ct, readtcp, writetcp);
	h->cl_ops = &tcp_ops;
	h->cl_private = (caddr_t)ct;
	h->cl_auth = authnone_create();
	return h;

fooy:
	mem_free((caddr_t)ct, sizeof(struct ct_data));
	mem_free((caddr_t)h, sizeof(CLIENT));
	return NULL;
}

CLIENT *
clnttcp_create(raddr, prog, vers, sockp, sendsz, recvsz)
#ifdef __LP64__
	struct sockaddr_in *raddr;
	uint32_t prog;
	uint32_t vers;
	int *sockp;
	uint32_t sendsz;
	uint32_t recvsz;
#else
	struct sockaddr_in *raddr;
	u_long prog;
	u_long vers;
	register int *sockp;
	u_int sendsz;
	u_int recvsz;
#endif
{
	return clnttcp_create_timeout(raddr, (uint32_t)prog, (uint32_t)vers, sockp, (uint32_t)sendsz, (uint32_t)recvsz, NULL, NULL);
}

static enum clnt_stat
clnttcp_call(h, proc, xdr_args, args_ptr, xdr_results, results_ptr, timeout)
	register CLIENT *h;
#ifdef __LP64__
	uint32_t proc;
#else
	u_long proc;
#endif
	xdrproc_t xdr_args;
	caddr_t args_ptr;
	xdrproc_t xdr_results;
	caddr_t results_ptr;
	struct timeval timeout;
{
	register struct ct_data *ct = (struct ct_data *) h->cl_private;
	register XDR *xdrs = &(ct->ct_xdrs);
	struct rpc_msg reply_msg;
#ifdef __LP64__
	uint32_t x_id;
	uint32_t *msg_x_id = (uint32_t *)(ct->ct_mcall);	/* yuk */
#else
	u_long x_id;
	u_long *msg_x_id = (u_long *)(ct->ct_mcall);	/* yuk */
#endif
	register bool_t shipnow;
	int refreshes = 2;

	if (!ct->ct_timeout_set) {
		ct->ct_timeout = timeout;
	}

	shipnow =
	    (xdr_results == (xdrproc_t)0 && timeout.tv_sec == 0
	    && timeout.tv_usec == 0) ? FALSE : TRUE;

call_again:
	xdrs->x_op = XDR_ENCODE;
	ct->ct_error.re_status = RPC_SUCCESS;
	x_id = ntohl(--(*msg_x_id));
#ifdef __LP64__
	if ((! XDR_PUTBYTES(xdrs, ct->ct_mcall, ct->ct_mpos)) ||
	    (! XDR_PUTLONG(xdrs, (int *)&proc)) ||
	    (! AUTH_MARSHALL(h->cl_auth, xdrs)) ||
	    (! (*xdr_args)(xdrs, args_ptr, 0)))
#else
	if ((! XDR_PUTBYTES(xdrs, ct->ct_mcall, ct->ct_mpos)) ||
		(! XDR_PUTLONG(xdrs, (long *)&proc)) ||
		(! AUTH_MARSHALL(h->cl_auth, xdrs)) ||
		(! (*xdr_args)(xdrs, args_ptr, 0)))
#endif			
	{
		if (ct->ct_error.re_status == RPC_SUCCESS)
			ct->ct_error.re_status = RPC_CANTENCODEARGS;
		(void)xdrrec_endofrecord(xdrs, TRUE);
		return (ct->ct_error.re_status);
	}
	if (! xdrrec_endofrecord(xdrs, shipnow))
		return (ct->ct_error.re_status = RPC_CANTSEND);
	if (! shipnow)
		return (RPC_SUCCESS);
	/*
	 * Hack to provide rpc-based message passing
	 */
	if (timeout.tv_sec == 0 && timeout.tv_usec == 0) {
		return(ct->ct_error.re_status = RPC_TIMEDOUT);
	}


	/*
	 * Keep receiving until we get a valid transaction id
	 */
	xdrs->x_op = XDR_DECODE;
	while (TRUE) {
		reply_msg.acpted_rply.ar_verf = _null_auth;
		reply_msg.acpted_rply.ar_results.where = NULL;
		reply_msg.acpted_rply.ar_results.proc = (xdrproc_t)xdr_void;
		if (! xdrrec_skiprecord(xdrs))
			return (ct->ct_error.re_status);
		/* now decode and validate the response header */
		if (! xdr_replymsg(xdrs, &reply_msg)) {
			if (ct->ct_error.re_status == RPC_SUCCESS)
				continue;
			return (ct->ct_error.re_status);
		}
		if (reply_msg.rm_xid == x_id)
			break;
	}

	/*
	 * process header
	 */
	_seterr_reply(&reply_msg, &(ct->ct_error));
	if (ct->ct_error.re_status == RPC_SUCCESS) {
		if (! AUTH_VALIDATE(h->cl_auth, &reply_msg.acpted_rply.ar_verf)) {
			ct->ct_error.re_status = RPC_AUTHERROR;
			ct->ct_error.re_why = AUTH_INVALIDRESP;
		} else if (! (*xdr_results)(xdrs, results_ptr, 0)) {
			if (ct->ct_error.re_status == RPC_SUCCESS)
				ct->ct_error.re_status = RPC_CANTDECODERES;
		}
		/* free verifier ... */
		if (reply_msg.acpted_rply.ar_verf.oa_base != NULL) {
			xdrs->x_op = XDR_FREE;
			(void)xdr_opaque_auth(xdrs, &(reply_msg.acpted_rply.ar_verf));
		}
	}  /* end successful completion */
	else {
		/* maybe our credentials need to be refreshed ... */
		if (refreshes-- && AUTH_REFRESH(h->cl_auth))
			goto call_again;
	}  /* end of unsuccessful completion */
	return (ct->ct_error.re_status);
}

static void
clnttcp_geterr(h, errp)
	CLIENT *h;
	struct rpc_err *errp;
{
	register struct ct_data *ct =
	    (struct ct_data *) h->cl_private;

	*errp = ct->ct_error;
}

static bool_t
clnttcp_freeres(cl, xdr_res, res_ptr)
	CLIENT *cl;
	xdrproc_t xdr_res;
	caddr_t res_ptr;
{
	register struct ct_data *ct = (struct ct_data *)cl->cl_private;
	register XDR *xdrs = &(ct->ct_xdrs);

	xdrs->x_op = XDR_FREE;
	return ((*xdr_res)(xdrs, res_ptr, 0));
}

static void
clnttcp_abort()
{
}

static bool_t
clnttcp_control(cl, request, info)
	CLIENT *cl;
	int request;
	char *info;
{
	register struct ct_data *ct = (struct ct_data *)cl->cl_private;

	switch (request) {
	case CLSET_TIMEOUT:
		ct->ct_timeout = *(struct timeval *)info;
		ct->ct_timeout_set = TRUE;
		break;
	case CLGET_TIMEOUT:
		*(struct timeval *)info = ct->ct_timeout;
		break;
	case CLGET_SERVER_ADDR:
		*(struct sockaddr_in *)info = ct->ct_addr;
		break;
	default:
		return (FALSE);
	}
	return (TRUE);
}


static void
clnttcp_destroy(h)
	CLIENT *h;
{
	register struct ct_data *ct =
	    (struct ct_data *) h->cl_private;

	if (ct->ct_closeit) {
		(void)close(ct->ct_sock);
	}
	XDR_DESTROY(&(ct->ct_xdrs));
	mem_free((caddr_t)ct, sizeof(struct ct_data));
	mem_free((caddr_t)h, sizeof(CLIENT));
}

/*
 * Interface between xdr serializer and tcp connection.
 * Behaves like the system calls, read & write, but keeps some error state
 * around for the rpc level.
 */
static int
readtcp(ct, buf, len)
	register struct ct_data *ct;
	caddr_t buf;
	register int len;
{
	fd_set mask;
	fd_set readfds;

	if (len == 0)
		return (0);
	FD_ZERO(&mask);
	FD_SET(ct->ct_sock, &mask);
	while (TRUE) {
		readfds = mask;
		switch (select(ct->ct_sock+1, &readfds, NULL, NULL, &(ct->ct_timeout))) {
		case 0:
			ct->ct_error.re_status = RPC_TIMEDOUT;
			return (-1);

		case -1:
			if (errno == EINTR)
				continue;
			ct->ct_error.re_status = RPC_CANTRECV;
			ct->ct_error.re_errno = errno;
			return (-1);
		}
		break;
	}
	switch (len = read(ct->ct_sock, buf, len)) {

	case 0:
		/* premature eof */
		ct->ct_error.re_errno = ECONNRESET;
		ct->ct_error.re_status = RPC_CANTRECV;
		len = -1;  /* it's really an error */
		break;

	case -1:
		ct->ct_error.re_errno = errno;
		ct->ct_error.re_status = RPC_CANTRECV;
		break;
	}
	return (len);
}

static int
writetcp(ct, buf, len)
	struct ct_data *ct;
	caddr_t buf;
	int len;
{
	register int i, cnt;

	for (cnt = len; cnt > 0; cnt -= i, buf += i) {
		if ((i = write(ct->ct_sock, buf, cnt)) == -1) {
			ct->ct_error.re_errno = errno;
			ct->ct_error.re_status = RPC_CANTSEND;
			return (-1);
		}
	}
	return (len);
}
