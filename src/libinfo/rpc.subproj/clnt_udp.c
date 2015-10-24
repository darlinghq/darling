/*
 * Copyright (c) 1999-2007 Apple Inc. All rights reserved.
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
/*static char *sccsid = "from: @(#)clnt_udp.c 1.39 87/08/11 Copyr 1984 Sun Micro";*/
/*static char *sccsid = "from: @(#)clnt_udp.c	2.2 88/08/01 4.0 RPCSRC";*/
static char *rcsid = "$Id: clnt_udp.c,v 1.4 2002/03/15 22:07:49 majka Exp $";
#endif

/*
 * clnt_udp.c, Implements a UDP/IP based, client side RPC.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <errno.h>
#include <rpc/pmap_clnt.h>

extern int	bindresvport();
extern bool_t	xdr_opaque_auth();

extern u_short pmap_getport_timeout(struct sockaddr_in *address, uint32_t program, uint32_t version, uint32_t protocol, struct timeval *timeout, struct timeval *totaltimeout);

extern int errno;

/*
 * UDP bases client side rpc operations
 */
static enum clnt_stat	clntudp_call();
static void		clntudp_abort();
static void		clntudp_geterr();
static bool_t		clntudp_freeres();
static bool_t           clntudp_control();
static void		clntudp_destroy();

static struct clnt_ops udp_ops = {
	clntudp_call,
	clntudp_abort,
	clntudp_geterr,
	clntudp_freeres,
	clntudp_destroy,
	clntudp_control
};

/* 
 * Private data kept per client handle
 */
struct cu_data {
	int		   cu_sock;
	bool_t		   cu_closeit;
	struct sockaddr_in cu_raddr;
	int		   cu_rlen;
	struct timeval	   cu_retry_timeout;
	struct timeval     cu_total_timeout;
	struct rpc_err	   cu_error;
	XDR		   cu_outxdrs;
	u_int		   cu_xdrpos;
	u_int		   cu_sendsz;
	char		   *cu_outbuf;
	u_int		   cu_recvsz;
	char		   cu_inbuf[1];
};

/*
 * Create a UDP based client handle.
 * If *sockp<0, *sockp is set to a newly created UPD socket.
 * If raddr->sin_port is 0 a binder on the remote machine
 * is consulted for the correct port number.
 * NB: It is the clients responsibility to close *sockp.
 * NB: The rpch->cl_auth is initialized to null authentication.
 *     Caller may wish to set this something more useful.
 *
 * wait is the amount of time used between retransmitting a call if
 * no response has been heard;  retransmition occurs until the actual
 * rpc call times out.
 *
 * sendsz and recvsz are the maximum allowable packet sizes that can be
 * sent and received.
 */
CLIENT *
clntudp_bufcreate_timeout(struct sockaddr_in *raddr, uint32_t program, uint32_t version, int *sockp, uint32_t sendsz, uint32_t recvsz, struct timeval *retry_timeout, struct timeval *total_timeout)
{
	CLIENT *cl;
	struct cu_data *cu = NULL;
	struct timeval now;
	struct rpc_msg call_msg;
	int rfd;
	u_short port;
	socklen_t len;
	unsigned int rsize;

	cl = (CLIENT *)mem_alloc(sizeof(CLIENT));
	if (cl == NULL)
	{
		rpc_createerr.cf_stat = RPC_SYSTEMERROR;
		rpc_createerr.cf_error.re_errno = errno;
		goto fooy;
	}

	sendsz = ((sendsz + 3) / 4) * 4;
	recvsz = ((recvsz + 3) / 4) * 4;
	cu = (struct cu_data *)mem_alloc(sizeof(*cu) + sendsz + recvsz);
	if (cu == NULL)
	{
		rpc_createerr.cf_stat = RPC_SYSTEMERROR;
		rpc_createerr.cf_error.re_errno = errno;
		goto fooy;
	}

	cu->cu_outbuf = &cu->cu_inbuf[recvsz];

	if (raddr->sin_port == 0)
	{
		port = pmap_getport_timeout(raddr, program, version, IPPROTO_UDP, retry_timeout, total_timeout);
		if (port == 0) goto fooy;

		raddr->sin_port = htons(port);
	}

	cl->cl_ops = &udp_ops;
	cl->cl_private = (caddr_t)cu;
	cu->cu_raddr = *raddr;
	cu->cu_rlen = sizeof (cu->cu_raddr);
	cu->cu_sendsz = sendsz;
	cu->cu_recvsz = recvsz;

	cu->cu_retry_timeout.tv_sec = 1;
	cu->cu_retry_timeout.tv_usec = 0;
	if (retry_timeout != NULL) cu->cu_retry_timeout = *retry_timeout;
	
	cu->cu_total_timeout.tv_sec = -1;
	cu->cu_total_timeout.tv_usec = -1;
	if (total_timeout != NULL) cu->cu_total_timeout = *total_timeout;
	
	rfd = open("/dev/random", O_RDONLY, 0);
	if ((rfd < 0) || (read(rfd, &call_msg.rm_xid, sizeof(call_msg.rm_xid)) != sizeof(call_msg.rm_xid)))
	{
		gettimeofday(&now, (struct timezone *)0);
		call_msg.rm_xid = getpid() ^ now.tv_sec ^ now.tv_usec;
	}

	if (rfd > 0) close(rfd);

	call_msg.rm_direction = CALL;
	call_msg.rm_call.cb_rpcvers = RPC_MSG_VERSION;
	call_msg.rm_call.cb_prog = program;
	call_msg.rm_call.cb_vers = version;
	xdrmem_create(&(cu->cu_outxdrs), cu->cu_outbuf, sendsz, XDR_ENCODE);
	if (! xdr_callhdr(&(cu->cu_outxdrs), &call_msg)) goto fooy;

	cu->cu_xdrpos = XDR_GETPOS(&(cu->cu_outxdrs));
	if (*sockp < 0)
	{
		int dontblock = 1;

		*sockp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (*sockp < 0)
		{
			rpc_createerr.cf_stat = RPC_SYSTEMERROR;
			rpc_createerr.cf_error.re_errno = errno;
			goto fooy;
		}

		/* attempt to bind to prov port */
		(void)bindresvport(*sockp, (struct sockaddr_in *)0);

		/* the socket's rpc controls are non-blocking */
		(void)ioctl(*sockp, FIONBIO, (char *) &dontblock);
		cu->cu_closeit = TRUE;

		/* set receive size */
		rsize = 0;
		len = sizeof(rsize);
		if (getsockopt(*sockp, SOL_SOCKET, SO_RCVBUF, (char *)&rsize, &len) != 0)
		{
			close(*sockp);
			*sockp = -1;
			goto fooy;
		}

		len = sizeof(recvsz);
		if ((recvsz > rsize) && (setsockopt(*sockp, SOL_SOCKET, SO_RCVBUF, (char *)&recvsz, len)))
		{
			close(*sockp);
			*sockp = -1;
			goto fooy;
		}
	}
	else
	{
		cu->cu_closeit = FALSE;
	}

	cu->cu_sock = *sockp;
	cl->cl_auth = authnone_create();
	return (cl);

fooy:
	if (cu) mem_free((caddr_t)cu, sizeof(*cu) + sendsz + recvsz);
	if (cl) mem_free((caddr_t)cl, sizeof(CLIENT));
	return NULL;
}

CLIENT *
clntudp_bufcreate(raddr, program, version, wait, sockp, sendsz, recvsz)
#ifdef __LP64__
	struct sockaddr_in *raddr;
	uint32_t program;
	uint32_t version;
	struct timeval wait;
	int *sockp;
	uint32_t sendsz;
	uint32_t recvsz;
#else
	struct sockaddr_in *raddr;
	u_long program;
	u_long version;
	struct timeval wait;
	register int *sockp;
	u_int sendsz;
	u_int recvsz;
#endif
{
	return clntudp_bufcreate_timeout(raddr, (uint32_t)program, (uint32_t)version, sockp, (uint32_t)sendsz, (uint32_t)recvsz, &wait, NULL);
}

CLIENT *
clntudp_create(raddr, program, version, wait, sockp)
#ifdef __LP64__
	struct sockaddr_in *raddr;
	uint32_t program;
	uint32_t version;
	struct timeval wait;
	int *sockp;
#else
	struct sockaddr_in *raddr;
	u_long program;
	u_long version;
	struct timeval wait;
	register int *sockp;
#endif
{

	return clntudp_bufcreate(raddr, program, version, wait, sockp, UDPMSGSIZE, UDPMSGSIZE);
}

static enum clnt_stat 
clntudp_call(cl, proc, xargs, argsp, xresults, resultsp, utimeout)
	register CLIENT	*cl;		/* client handle */
#ifdef __LP64__
	uint32_t		proc;		/* procedure number */
#else
	u_long		proc;		/* procedure number */
#endif
	xdrproc_t	xargs;		/* xdr routine for args */
	caddr_t		argsp;		/* pointer to args */
	xdrproc_t	xresults;	/* xdr routine for results */
	caddr_t		resultsp;	/* pointer to results */
	struct timeval	utimeout;	/* seconds to wait before giving up */
{
	register struct cu_data *cu = (struct cu_data *)cl->cl_private;
	register XDR *xdrs;
	register int outlen;
	register int inlen;
	u_int32_t fromlen;
	fd_set readfds;
	fd_set mask;
	struct sockaddr_in from;
	struct rpc_msg reply_msg;
	XDR reply_xdrs;
	struct timeval time_waited;
	bool_t ok;
	int nrefreshes = 2;	/* number of times to refresh cred */
	struct timeval timeout;

	if (cu->cu_total_timeout.tv_sec == -1)
	{
		/* use supplied total timeout */
		timeout = utimeout;
	}
	else
	{
		/* use client's total timeout */
		timeout = cu->cu_total_timeout;
	}

	time_waited.tv_sec = 0;
	time_waited.tv_usec = 0;

call_again:
	xdrs = &(cu->cu_outxdrs);
	xdrs->x_op = XDR_ENCODE;
	XDR_SETPOS(xdrs, cu->cu_xdrpos);

	/*
	 * the transaction is the first thing in the out buffer
	 */
	(*(u_short *)(cu->cu_outbuf))++;
#ifdef __LP64__
	if ((! XDR_PUTLONG(xdrs, (int *)&proc)) ||
	    (! AUTH_MARSHALL(cl->cl_auth, xdrs)) ||
	    (! (*xargs)(xdrs, argsp, 0)))
		return (cu->cu_error.re_status = RPC_CANTENCODEARGS);
#else
	if ((! XDR_PUTLONG(xdrs, (long *)&proc)) ||
	    (! AUTH_MARSHALL(cl->cl_auth, xdrs)) ||
	    (! (*xargs)(xdrs, argsp, 0)))
		return (cu->cu_error.re_status = RPC_CANTENCODEARGS);
#endif
	outlen = (int)XDR_GETPOS(xdrs);

send_again:
	if (sendto(cu->cu_sock, cu->cu_outbuf, outlen, 0, (struct sockaddr *)&(cu->cu_raddr), cu->cu_rlen) != outlen)
	{
		cu->cu_error.re_errno = errno;
		return (cu->cu_error.re_status = RPC_CANTSEND);
	}

	/*
	 * Hack to provide rpc-based message passing
	 */
	if (timeout.tv_sec == 0 && timeout.tv_usec == 0)
	{
		return (cu->cu_error.re_status = RPC_TIMEDOUT);
	}

	/*
	 * sub-optimal code appears here because we have
	 * some clock time to spare while the packets are in flight.
	 * (We assume that this is actually only executed once.)
	 */
	reply_msg.acpted_rply.ar_verf = _null_auth;
	reply_msg.acpted_rply.ar_results.where = resultsp;
	reply_msg.acpted_rply.ar_results.proc = xresults;
	FD_ZERO(&mask);
	FD_SET(cu->cu_sock, &mask);
	for (;;)
	{
		readfds = mask;
		switch (select(cu->cu_sock+1, &readfds, NULL, NULL, &(cu->cu_retry_timeout))) {
		case 0:
			time_waited.tv_sec += cu->cu_retry_timeout.tv_sec;
			time_waited.tv_usec += cu->cu_retry_timeout.tv_usec;
			while (time_waited.tv_usec >= 1000000) {
				time_waited.tv_sec++;
				time_waited.tv_usec -= 1000000;
			}
			if ((time_waited.tv_sec < timeout.tv_sec) ||
				((time_waited.tv_sec == timeout.tv_sec) &&
				(time_waited.tv_usec < timeout.tv_usec)))
				goto send_again;	
			return (cu->cu_error.re_status = RPC_TIMEDOUT);

		/*
		 * buggy in other cases because time_waited is not being
		 * updated.
		 */
		case -1:
			if (errno == EINTR)
				continue;	
			cu->cu_error.re_errno = errno;
			return (cu->cu_error.re_status = RPC_CANTRECV);
		}
		do {
			fromlen = sizeof(struct sockaddr);
			inlen = recvfrom(cu->cu_sock, cu->cu_inbuf, 
				(int) cu->cu_recvsz, 0,
				(struct sockaddr *)&from, &fromlen);
		} while (inlen < 0 && errno == EINTR);
		if (inlen < 0) {
			if (errno == EWOULDBLOCK)
				continue;	
			cu->cu_error.re_errno = errno;
			return (cu->cu_error.re_status = RPC_CANTRECV);
		}
#ifdef __LP64__
		if (inlen < sizeof(uint32_t))
			continue;
		/* see if reply transaction id matches sent id */
		if (*((uint32_t *)(cu->cu_inbuf)) != *((uint32_t *)(cu->cu_outbuf))) continue;
#else
		if (inlen < sizeof(u_long))
			continue;
		/* see if reply transaction id matches sent id */
		if (*((u_long *)(cu->cu_inbuf)) != *((u_long *)(cu->cu_outbuf))) continue;
#endif
		/* we now assume we have the proper reply */
		break;
	}

	/*
	 * now decode and validate the response
	 */
	xdrmem_create(&reply_xdrs, cu->cu_inbuf, (u_int)inlen, XDR_DECODE);
	ok = xdr_replymsg(&reply_xdrs, &reply_msg);
	/* XDR_DESTROY(&reply_xdrs);  save a few cycles on noop destroy */
	if (ok) {
		_seterr_reply(&reply_msg, &(cu->cu_error));
		if (cu->cu_error.re_status == RPC_SUCCESS) {
			if (! AUTH_VALIDATE(cl->cl_auth,
				&reply_msg.acpted_rply.ar_verf)) {
				cu->cu_error.re_status = RPC_AUTHERROR;
				cu->cu_error.re_why = AUTH_INVALIDRESP;
			}
			if (reply_msg.acpted_rply.ar_verf.oa_base != NULL) {
				xdrs->x_op = XDR_FREE;
				(void)xdr_opaque_auth(xdrs,
				    &(reply_msg.acpted_rply.ar_verf));
			} 
		}  /* end successful completion */
		else {
			/* maybe our credentials need to be refreshed ... */
			if (nrefreshes > 0 && AUTH_REFRESH(cl->cl_auth)) {
				nrefreshes--;
				goto call_again;
			}
		}  /* end of unsuccessful completion */
	}  /* end of valid reply message */
	else {
		cu->cu_error.re_status = RPC_CANTDECODERES;
	}
	return (cu->cu_error.re_status);
}

static void
clntudp_geterr(cl, errp)
	CLIENT *cl;
	struct rpc_err *errp;
{
	register struct cu_data *cu = (struct cu_data *)cl->cl_private;

	*errp = cu->cu_error;
}


static bool_t
clntudp_freeres(cl, xdr_res, res_ptr)
	CLIENT *cl;
	xdrproc_t xdr_res;
	caddr_t res_ptr;
{
	register struct cu_data *cu = (struct cu_data *)cl->cl_private;
	register XDR *xdrs = &(cu->cu_outxdrs);

	xdrs->x_op = XDR_FREE;
	return ((*xdr_res)(xdrs, res_ptr, 0));
}

static void 
clntudp_abort(/*h*/)
	/*CLIENT *h;*/
{
}

static bool_t
clntudp_control(cl, request, info)
	CLIENT *cl;
	int request;
	char *info;
{
	register struct cu_data *cu = (struct cu_data *)cl->cl_private;

	switch (request) {
	case CLSET_TIMEOUT:
		cu->cu_total_timeout = *(struct timeval *)info;
		break;
	case CLGET_TIMEOUT:
		*(struct timeval *)info = cu->cu_total_timeout;
		break;
	case CLSET_RETRY_TIMEOUT:
		cu->cu_retry_timeout = *(struct timeval *)info;
		break;
	case CLGET_RETRY_TIMEOUT:
		*(struct timeval *)info = cu->cu_retry_timeout;
		break;
	case CLGET_SERVER_ADDR:
		*(struct sockaddr_in *)info = cu->cu_raddr;
		break;
	default:
		return (FALSE);
	}
	return (TRUE);
}
	
static void
clntudp_destroy(cl)
	CLIENT *cl;
{
	register struct cu_data *cu = (struct cu_data *)cl->cl_private;

	if (cu->cu_closeit) {
		(void)close(cu->cu_sock);
	}
	XDR_DESTROY(&(cu->cu_outxdrs));
	mem_free((caddr_t)cu, (sizeof(*cu) + cu->cu_sendsz + cu->cu_recvsz));
	mem_free((caddr_t)cl, sizeof(CLIENT));
}
