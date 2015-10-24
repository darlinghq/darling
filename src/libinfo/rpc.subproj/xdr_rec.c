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

/*	$NetBSD: xdr_rec.c,v 1.18 2000/07/06 03:10:35 christos Exp $	*/

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

#include <sys/cdefs.h>
#if defined(LIBC_SCCS) && !defined(lint) 
static char *sccsid = "@(#)xdr_rec.c 1.21 87/08/11 Copyr 1984 Sun Micro";
static char *sccsid = "@(#)xdr_rec.c	2.2 88/08/01 4.0 RPCSRC";
#endif
#include <sys/cdefs.h>

/*
 * xdr_rec.c, Implements TCP/IP based XDR streams with a "record marking"
 * layer above tcp (for rpc's use).
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * These routines interface XDRSTREAMS to a tcp/ip connection.
 * There is a record marking layer between the xdr stream
 * and the tcp transport level.  A record is composed on one or more
 * record fragments.  A record fragment is a thirty-two bit header followed
 * by n bytes of data, where n is contained in the header.  The header
 * is in network byte order.  Thegh order bit encodes
 * whether or not the fragment is the last fragment of the record
 * (1 => fragment is last, 0 => more fragments to follow. 
 * The other 31 bits encode the byte length of the fragment.
 */

#include <sys/types.h>

#include <netinet/in.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/svc.h>
#include <rpc/clnt.h>

#ifdef __LP64__
static bool_t	xdrrec_getlong(XDR *, int *);
static bool_t	xdrrec_putlong(XDR *, const int *);
#else
static bool_t	xdrrec_getlong(XDR *, long *);
static bool_t	xdrrec_putlong(XDR *, const long *);
#endif

static bool_t	xdrrec_getbytes(XDR *, char *, u_int);

static bool_t	xdrrec_putbytes(XDR *, const char *, u_int);
static u_int	xdrrec_getpos(XDR *);
static bool_t	xdrrec_setpos(XDR *, u_int);
static int32_t *xdrrec_inline(XDR *, u_int);
static void	xdrrec_destroy(XDR *);

bool_t __xdrrec_getrec(XDR *, enum xprt_stat *, bool_t);

static const struct  xdr_ops xdrrec_ops = {
	xdrrec_getlong,
	xdrrec_putlong,
	xdrrec_getbytes,
	xdrrec_putbytes,
	xdrrec_getpos,
	xdrrec_setpos,
	xdrrec_inline,
	xdrrec_destroy
};

/*
 * A record is composed of one or more record fragments.
 * A record fragment is a four-byte header followed by zero to
 * 2**32-1 bytes.  The header is treated as an unsigned 32-bit integer and is
 * encode/decoded to the network via htonl/ntohl.  The low order 31 bits
 * are a byte count of the fragment.  The highest order bit is a boolean:
 * 1 => this fragment is the last fragment of the record,
 * 0 => this fragment is followed by more fragment(s).
 *
 * The fragment/record machinery is not general;  it is constructed to
 * meet the needs of xdr and rpc based on tcp.
 */

#define LAST_FRAG ((u_int32_t)(1 << 31))

typedef struct rec_strm {
	void *tcp_handle;
	/*
	 * out-goung bits
	 */
	int (*writeit)(void *, void *, int);
	char *out_base;	/* output buffer (points to frag header) */
	char *out_finger;	/* next output position */
	char *out_boundry;	/* data cannot up to this address */
	u_int32_t *frag_header;	/* beginning of curren fragment */
	bool_t frag_sent;	/* true if buffer sent in middle of record */
	/*
	 * in-coming bits
	 */
	int (*readit)(void *, void *, int);
	size_t in_size;	/* fixed size of the input buffer */
	char *in_base;
	char *in_finger;	/* location of next byte to be had */
	char *in_boundry;	/* can read up to this location */
	int fbtbc;		/* fragment bytes to be consumed */
	bool_t last_frag;
	u_int sendsize;
	u_int recvsize;

	bool_t nonblock;
	bool_t in_haveheader;
	u_int32_t in_header;
	char *in_hdrp;
	int in_hdrlen;
	int in_reclen;
	int in_received;
	int in_maxrec;
} RECSTREAM;

static u_int	fix_buf_size(u_int);
static bool_t	flush_out(RECSTREAM *, bool_t);
static bool_t	fill_input_buf(RECSTREAM *);
static bool_t	get_input_bytes(RECSTREAM *, char *, int);
static bool_t	set_input_fragment(RECSTREAM *);
static bool_t	skip_input_bytes(RECSTREAM *, int);
static bool_t	realloc_stream(RECSTREAM *, int);


/*
 * Create an xdr handle for xdrrec
 * xdrrec_create fills in xdrs.  Sendsize and recvsize are
 * send and recv buffer sizes (0 => use default).
 * tcp_handle is an opaque handle that is passed as the first parameter to
 * the procedures readit and writeit.  Readit and writeit are read and
 * write respectively.   They are like the system
 * calls expect that they take an opaque handle rather than an fd.
 */
void
xdrrec_create(xdrs, sendsize, recvsize, tcp_handle, readit, writeit)
	XDR *xdrs;
	u_int sendsize;
	u_int recvsize;
	void *tcp_handle;
	/* like read, but pass it a tcp_handle, not sock */
	int (*readit)(void *, void *, int);
	/* like write, but pass it a tcp_handle, not sock */
	int (*writeit)(void *, void *, int);
{
	RECSTREAM *rstrm = mem_alloc(sizeof(RECSTREAM));

	if (rstrm == NULL) {
		warnx("xdrrec_create: out of memory");
		/* 
		 *  This is bad.  Should rework xdrrec_create to 
		 *  return a handle, and in this case return NULL
		 */
		return;
	}
	rstrm->sendsize = sendsize = fix_buf_size(sendsize);
	rstrm->out_base = mem_alloc(rstrm->sendsize);
	if (rstrm->out_base == NULL) {
		warnx("xdrrec_create: out of memory");
		mem_free(rstrm, sizeof(RECSTREAM));
		return;
	}
	rstrm->recvsize = recvsize = fix_buf_size(recvsize);
	rstrm->in_base = mem_alloc(recvsize);
	if (rstrm->in_base == NULL) {
		warnx("xdrrec_create: out of memory");
		mem_free(rstrm->out_base, sendsize);
		mem_free(rstrm, sizeof(RECSTREAM));
		return;
	}
	/*
	 * now the rest ...
	 */
	xdrs->x_ops = &xdrrec_ops;
	xdrs->x_private = rstrm;
	rstrm->tcp_handle = tcp_handle;
	rstrm->readit = readit;
	rstrm->writeit = writeit;
	rstrm->out_finger = rstrm->out_boundry = rstrm->out_base;
	rstrm->frag_header = (u_int32_t *)(void *)rstrm->out_base;
	rstrm->out_finger += sizeof(u_int32_t);
	rstrm->out_boundry += sendsize;
	rstrm->frag_sent = FALSE;
	rstrm->in_size = recvsize;
	rstrm->in_boundry = rstrm->in_base;
	rstrm->in_finger = (rstrm->in_boundry += recvsize);
	rstrm->fbtbc = 0;
	rstrm->last_frag = TRUE;
	rstrm->in_haveheader = FALSE;
	rstrm->in_hdrlen = 0;
	rstrm->in_hdrp = (char *)(void *)&rstrm->in_header;
	rstrm->nonblock = FALSE;
	rstrm->in_reclen = 0;
	rstrm->in_received = 0;
}


/*
 * The reoutines defined below are the xdr ops which will go into the
 * xdr handle filled in by xdrrec_create.
 */

static bool_t
xdrrec_getlong(xdrs, lp)
	XDR *xdrs;
#ifdef __LP64__
	int *lp;
#else
	long *lp;
#endif
{
	RECSTREAM *rstrm = (RECSTREAM *)(xdrs->x_private);
	int32_t *buflp = (int32_t *)(void *)(rstrm->in_finger);
	int32_t mylong;

	/* first try the inline, fast case */
	if ((rstrm->fbtbc >= sizeof(int32_t)) && ((rstrm->in_boundry - (char *)buflp) >= sizeof(int32_t)))
	{
		*lp = ntohl(*buflp);
		rstrm->fbtbc -= sizeof(int32_t);
		rstrm->in_finger += sizeof(int32_t);
	}
	else
	{
		if (! xdrrec_getbytes(xdrs, (char *)(void *)&mylong, sizeof(int32_t))) return (FALSE);
		*lp = ntohl(mylong);
	}

	return (TRUE);
}

static bool_t
xdrrec_putlong(xdrs, lp)
	XDR *xdrs;
#ifdef __LP64__
	const int *lp;
#else
	const long *lp;
#endif
{
	RECSTREAM *rstrm = (RECSTREAM *)(xdrs->x_private);
	int32_t *dest_lp = ((int32_t *)(void *)(rstrm->out_finger));

	if ((rstrm->out_finger += sizeof(int32_t)) > rstrm->out_boundry)
	{
		/*
		 * this case should almost never happen so the code is
		 * inefficient
		 */
		rstrm->out_finger -= sizeof(int32_t);
		rstrm->frag_sent = TRUE;
		if (! flush_out(rstrm, FALSE)) return (FALSE);
		dest_lp = ((int32_t *)(void *)(rstrm->out_finger));
		rstrm->out_finger += sizeof(int32_t);
	}

	*dest_lp = htonl(*lp);
	return (TRUE);
}

static bool_t  /* must manage buffers, fragments, and records */
xdrrec_getbytes(xdrs, addr, len)
	XDR *xdrs;
	char *addr;
	u_int len;
{
	RECSTREAM *rstrm = (RECSTREAM *)(xdrs->x_private);
	int current;

	while (len > 0) {
		current = (int)rstrm->fbtbc;
		if (current == 0) {
			if (rstrm->last_frag)
				return (FALSE);
			if (! set_input_fragment(rstrm))
				return (FALSE);
			continue;
		}
		current = (len < current) ? len : current;
		if (! get_input_bytes(rstrm, addr, current))
			return (FALSE);
		addr += current; 
		rstrm->fbtbc -= current;
		len -= current;
	}
	return (TRUE);
}

static bool_t
xdrrec_putbytes(xdrs, addr, len)
	XDR *xdrs;
	const char *addr;
	u_int len;
{
	RECSTREAM *rstrm = (RECSTREAM *)(xdrs->x_private);
	size_t current;

	while (len > 0) {
		current = (size_t)(rstrm->out_boundry - rstrm->out_finger);
		current = (len < current) ? len : current;
		memmove(rstrm->out_finger, addr, current);
		rstrm->out_finger += current;
		addr += current;
		len -= current;
		if (rstrm->out_finger == rstrm->out_boundry) {
			rstrm->frag_sent = TRUE;
			if (! flush_out(rstrm, FALSE))
				return (FALSE);
		}
	}
	return (TRUE);
}

static u_int
xdrrec_getpos(xdrs)
	XDR *xdrs;
{
	RECSTREAM *rstrm = (RECSTREAM *)xdrs->x_private;
	off_t pos;
	int hfd;

	/* tcp_handle is in actual fact just a file descriptor */
	hfd = 0;
	memcpy(&hfd, rstrm->tcp_handle, sizeof(hfd));

	pos = lseek(hfd, 0, 1);
	if (pos != -1)
		switch (xdrs->x_op) {

		case XDR_ENCODE:
			pos += rstrm->out_finger - rstrm->out_base;
			break;

		case XDR_DECODE:
			pos -= rstrm->in_boundry - rstrm->in_finger;
			break;

		default:
			pos = (off_t) -1;
			break;
		}
	return ((u_int) pos);
}

static bool_t
xdrrec_setpos(xdrs, pos)
	XDR *xdrs;
	u_int pos;
{
	RECSTREAM *rstrm = (RECSTREAM *)xdrs->x_private;
	u_int currpos = xdrrec_getpos(xdrs);
	int delta = currpos - pos;
	char *newpos;

	if ((int)currpos != -1)
		switch (xdrs->x_op) {

		case XDR_ENCODE:
			newpos = rstrm->out_finger - delta;
			if ((newpos > (char *)(void *)(rstrm->frag_header)) &&
				(newpos < rstrm->out_boundry)) {
				rstrm->out_finger = newpos;
				return (TRUE);
			}
			break;

		case XDR_DECODE:
			newpos = rstrm->in_finger - delta;
			if ((delta < (int)(rstrm->fbtbc)) &&
				(newpos <= rstrm->in_boundry) &&
				(newpos >= rstrm->in_base)) {
				rstrm->in_finger = newpos;
				rstrm->fbtbc -= delta;
				return (TRUE);
			}
			break;

		case XDR_FREE:
			break;
		}
	return (FALSE);
}

static int32_t *
xdrrec_inline(xdrs, len)
	XDR *xdrs;
	u_int len;
{
	RECSTREAM *rstrm = (RECSTREAM *)xdrs->x_private;
	int32_t *buf = NULL;

	switch (xdrs->x_op) {

	case XDR_ENCODE:
		if ((rstrm->out_finger + len) <= rstrm->out_boundry) {
			buf = (int32_t *)(void *)rstrm->out_finger;
			rstrm->out_finger += len;
		}
		break;

	case XDR_DECODE:
		if ((len <= rstrm->fbtbc) &&
			((rstrm->in_finger + len) <= rstrm->in_boundry)) {
			buf = (int32_t *)(void *)rstrm->in_finger;
			rstrm->fbtbc -= len;
			rstrm->in_finger += len;
		}
		break;

	case XDR_FREE:
		break;
	}
	return (buf);
}

static void
xdrrec_destroy(xdrs)
	XDR *xdrs;
{
	RECSTREAM *rstrm = (RECSTREAM *)xdrs->x_private;

	mem_free(rstrm->out_base, rstrm->sendsize);
	mem_free(rstrm->in_base, rstrm->recvsize);
	mem_free(rstrm, sizeof(RECSTREAM));
}


/*
 * Exported routines to manage xdr records
 */

/*
 * Before reading (deserializing from the stream, one should always call
 * this procedure to guarantee proper record alignment.
 */
bool_t
xdrrec_skiprecord(xdrs)
	XDR *xdrs;
{
	RECSTREAM *rstrm = (RECSTREAM *)(xdrs->x_private);
	enum xprt_stat xstat;

	if (rstrm->nonblock) {
		if (__xdrrec_getrec(xdrs, &xstat, FALSE)) {
			rstrm->fbtbc = 0;
			return TRUE;
		}
		if (rstrm->in_finger == rstrm->in_boundry &&
		    xstat == XPRT_MOREREQS) {
			rstrm->fbtbc = 0;
			return TRUE;
		}
		return FALSE;
	}

	while (rstrm->fbtbc > 0 || (! rstrm->last_frag)) {
		if (! skip_input_bytes(rstrm, rstrm->fbtbc))
			return (FALSE);
		rstrm->fbtbc = 0;
		if ((! rstrm->last_frag) && (! set_input_fragment(rstrm)))
			return (FALSE);
	}
	rstrm->last_frag = FALSE;
	return (TRUE);
}

/*
 * Look ahead function.
 * Returns TRUE iff there is no more input in the buffer
 * after consuming the rest of the current record.
 */
bool_t
xdrrec_eof(xdrs)
	XDR *xdrs;
{
	RECSTREAM *rstrm = (RECSTREAM *)(xdrs->x_private);

	while (rstrm->fbtbc > 0 || (! rstrm->last_frag)) {
		if (! skip_input_bytes(rstrm, rstrm->fbtbc))
			return (TRUE);
		rstrm->fbtbc = 0;
		if ((! rstrm->last_frag) && (! set_input_fragment(rstrm)))
			return (TRUE);
	}
	if (rstrm->in_finger == rstrm->in_boundry)
		return (TRUE);
	return (FALSE);
}

/*
 * The client must tell the package when an end-of-record has occurred.
 * The second paraemters tells whether the record should be flushed to the
 * (output) tcp stream.  (This let's the package support batched or
 * pipelined procedure calls.)  TRUE => immmediate flush to tcp connection.
 */
bool_t
xdrrec_endofrecord(xdrs, sendnow)
	XDR *xdrs;
	bool_t sendnow;
{
	RECSTREAM *rstrm = (RECSTREAM *)(xdrs->x_private);
	unsigned int len;  /* fragment length */

	if (sendnow || rstrm->frag_sent ||
		(rstrm->out_finger + sizeof(u_int32_t) >= rstrm->out_boundry)) {
		rstrm->frag_sent = FALSE;
		return (flush_out(rstrm, TRUE));
	}
	len = rstrm->out_finger - (char *)(rstrm->frag_header) - sizeof(u_int32_t);
	*(rstrm->frag_header) = htonl(len | LAST_FRAG);
	rstrm->frag_header = (u_int32_t *)(void *)rstrm->out_finger;
	rstrm->out_finger += sizeof(u_int32_t);
	return (TRUE);
}

/*
 * Fill the stream buffer with a record for a non-blocking connection.
 * Return true if a record is available in the buffer, false if not.
 */
bool_t
__xdrrec_getrec(xdrs, statp, expectdata)
	XDR *xdrs;
	enum xprt_stat *statp;
	bool_t expectdata;
{
	RECSTREAM *rstrm = (RECSTREAM *)(xdrs->x_private);
	ssize_t n;
	int fraglen;

	if (!rstrm->in_haveheader) {
		n = rstrm->readit(rstrm->tcp_handle, rstrm->in_hdrp,
		    (int)sizeof (rstrm->in_header) - rstrm->in_hdrlen);
		if (n == 0) {
			*statp = expectdata ? XPRT_DIED : XPRT_IDLE;
			return FALSE;
		}
		if (n < 0) {
			*statp = XPRT_DIED;
			return FALSE;
		}
		rstrm->in_hdrp += n;
		rstrm->in_hdrlen += n;
		if (rstrm->in_hdrlen < sizeof (rstrm->in_header)) {
			*statp = XPRT_MOREREQS;
			return FALSE;
		}
		rstrm->in_header = ntohl(rstrm->in_header);
		fraglen = (int)(rstrm->in_header & ~LAST_FRAG);
		if (fraglen == 0 || fraglen > rstrm->in_maxrec ||
		    (rstrm->in_reclen + fraglen) > rstrm->in_maxrec) {
			*statp = XPRT_DIED;
			return FALSE;
		}
		rstrm->in_reclen += fraglen;
		if (rstrm->in_reclen > rstrm->recvsize)
			realloc_stream(rstrm, rstrm->in_reclen);
		if (rstrm->in_header & LAST_FRAG) {
			rstrm->in_header &= ~LAST_FRAG;
			rstrm->last_frag = TRUE;
		}
	}

	n =  rstrm->readit(rstrm->tcp_handle,
	    rstrm->in_base + rstrm->in_received,
	    (rstrm->in_reclen - rstrm->in_received));

	if (n < 0) {
		*statp = XPRT_DIED;
		return FALSE;
	}

	if (n == 0) {
		*statp = expectdata ? XPRT_DIED : XPRT_IDLE;
		return FALSE;
	}

	rstrm->in_received += n;

	if (rstrm->in_received == rstrm->in_reclen) {
		rstrm->in_haveheader = FALSE;
		rstrm->in_hdrp = (char *)(void *)&rstrm->in_header;
		rstrm->in_hdrlen = 0;
		if (rstrm->last_frag) {
			rstrm->fbtbc = rstrm->in_reclen;
			rstrm->in_boundry = rstrm->in_base + rstrm->in_reclen;
			rstrm->in_finger = rstrm->in_base;
			rstrm->in_reclen = rstrm->in_received = 0;
			*statp = XPRT_MOREREQS;
			return TRUE;
		}
	}

	*statp = XPRT_MOREREQS;
	return FALSE;
}

bool_t
__xdrrec_setnonblock(xdrs, maxrec)
	XDR *xdrs;
	int maxrec;
{
	RECSTREAM *rstrm = (RECSTREAM *)(xdrs->x_private);

	rstrm->nonblock = TRUE;
	if (maxrec == 0)
		maxrec = rstrm->recvsize;
	rstrm->in_maxrec = maxrec;
	return TRUE;
}

/*
 * Internal useful routines
 */
static bool_t
flush_out(rstrm, eor)
	RECSTREAM *rstrm;
	bool_t eor;
{
	u_int32_t eormask = (eor == TRUE) ? LAST_FRAG : 0;
	u_int32_t len = rstrm->out_finger - (char *)(rstrm->frag_header) - sizeof(u_int32_t);

	*(rstrm->frag_header) = htonl(len | eormask);
	len = rstrm->out_finger - rstrm->out_base;
	if ((*(rstrm->writeit))(rstrm->tcp_handle, rstrm->out_base, len) != len)
		return (FALSE);
	rstrm->frag_header = (u_int32_t *)(void *)rstrm->out_base;
	rstrm->out_finger = (char *)rstrm->out_base + sizeof(u_int32_t);
	return (TRUE);
}

static bool_t  /* knows nothing about records!  Only about input buffers */
fill_input_buf(rstrm)
	RECSTREAM *rstrm;
{
	char *where;
	u_int32_t i;
	int len;

	if (rstrm->nonblock)
		return FALSE;

	where = rstrm->in_base;
	i = (size_t)(rstrm->in_boundry) % BYTES_PER_XDR_UNIT;
	where += i;
	len = rstrm->in_size - i;
	if ((len = (*(rstrm->readit))(rstrm->tcp_handle, where, len)) == -1)
		return (FALSE);
	rstrm->in_finger = where;
	where += len;
	rstrm->in_boundry = where;
	return (TRUE);
}

static bool_t  /* knows nothing about records!  Only about input buffers */
get_input_bytes(rstrm, addr, len)
	RECSTREAM *rstrm;
	char *addr;
	int len;
{
	size_t current;

	if (rstrm->nonblock) {
		if (len > (int)(rstrm->in_boundry - rstrm->in_finger))
			return FALSE;
		memcpy(addr, rstrm->in_finger, (size_t)len);
		rstrm->in_finger += len;
		return TRUE;
	}

	while (len > 0) {
		current = (size_t)(rstrm->in_boundry - rstrm->in_finger);
		if (current == 0) {
			if (! fill_input_buf(rstrm))
				return (FALSE);
			continue;
		}
		current = (len < current) ? len : current;
		memmove(addr, rstrm->in_finger, current);
		rstrm->in_finger += current;
		addr += current;
		len -= current;
	}
	return (TRUE);
}

static bool_t  /* next two bytes of the input stream are treated as a header */
set_input_fragment(rstrm)
	RECSTREAM *rstrm;
{
	u_int32_t header;

	if (rstrm->nonblock)
		return FALSE;
	if (! get_input_bytes(rstrm, (char *)(void *)&header, sizeof(header)))
		return (FALSE);
	header = ntohl(header);
	rstrm->last_frag = ((header & LAST_FRAG) == 0) ? FALSE : TRUE;
	/*
	 * Sanity check. Try not to accept wildly incorrect
	 * record sizes. Unfortunately, the only record size
	 * we can positively identify as being 'wildly incorrect'
	 * is zero. Ridiculously large record sizes may look wrong,
	 * but we don't have any way to be certain that they aren't
	 * what the client actually intended to send us.
	 */
	if (header == 0)
		return(FALSE);
	rstrm->fbtbc = header & (~LAST_FRAG);
	return (TRUE);
}

static bool_t  /* consumes input bytes; knows nothing about records! */
skip_input_bytes(rstrm, cnt)
	RECSTREAM *rstrm;
	int cnt;
{
	u_int32_t current;

	while (cnt > 0) {
		current = (size_t)(rstrm->in_boundry - rstrm->in_finger);
		if (current == 0) {
			if (! fill_input_buf(rstrm))
				return (FALSE);
			continue;
		}
		current = (cnt < current) ? cnt : current;
		rstrm->in_finger += current;
		cnt -= current;
	}
	return (TRUE);
}

static u_int
fix_buf_size(s)
	u_int s;
{

	if (s < 100)
		s = 4000;
	return (RNDUP(s));
}

/*
 * Reallocate the input buffer for a non-block stream.
 */
static bool_t
realloc_stream(rstrm, size)
	RECSTREAM *rstrm;
	int size;
{
	int diff;
	char *buf;

	if (size > rstrm->recvsize) {
		buf = realloc(rstrm->in_base, (size_t)size);
		if (buf == NULL)
			return FALSE;
		diff = buf - rstrm->in_base;
		rstrm->in_finger += diff;
		rstrm->in_base = buf;
		rstrm->in_boundry = buf + size;
		rstrm->recvsize = size;
		rstrm->in_size = size;
	}

	return TRUE;
}
