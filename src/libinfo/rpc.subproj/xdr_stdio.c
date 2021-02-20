/*
 * Copyright (c) 1999-2018 Apple Inc. All rights reserved.
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

/*	$NetBSD: xdr_stdio.c,v 1.14 2000/01/22 22:19:19 mycroft Exp $	*/

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
static char *sccsid = "@(#)xdr_stdio.c 1.16 87/08/11 Copyr 1984 Sun Micro";
static char *sccsid = "@(#)xdr_stdio.c	2.1 88/07/29 4.0 RPCSRC";
#endif
#include "libinfo_common.h"

#include <sys/cdefs.h>

/*
 * xdr_stdio.c, XDR implementation on standard i/o file.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * This set of routines implements a XDR on a stdio stream.
 * XDR_ENCODE serializes onto the stream, XDR_DECODE de-serializes
 * from the stream.
 */

#include <stdio.h>

#include <arpa/inet.h>
#include <rpc/types.h>
#include <rpc/xdr.h>

static void xdrstdio_destroy(XDR *);
#ifdef __LP64__
static bool_t xdrstdio_getlong(XDR *, int *);
static bool_t xdrstdio_putlong(XDR *, const int *);
#else
static bool_t xdrstdio_getlong(XDR *, long *);
static bool_t xdrstdio_putlong(XDR *, const long *);
#endif
static bool_t xdrstdio_getbytes(XDR *, char *, u_int);
static bool_t xdrstdio_putbytes(XDR *, const char *, u_int);
static u_int xdrstdio_getpos(XDR *);
static bool_t xdrstdio_setpos(XDR *, u_int);
static int32_t *xdrstdio_inline(XDR *, u_int);

/*
 * Ops vector for stdio type XDR
 */
static const struct xdr_ops	xdrstdio_ops = {
	xdrstdio_getlong,	/* deseraialize a long int */
	xdrstdio_putlong,	/* seraialize a long int */
	xdrstdio_getbytes,	/* deserialize counted bytes */
	xdrstdio_putbytes,	/* serialize counted bytes */
	xdrstdio_getpos,	/* get offset in the stream */
	xdrstdio_setpos,	/* set offset in the stream */
	xdrstdio_inline,	/* prime stream for inline macros */
	xdrstdio_destroy	/* destroy stream */
};

/*
 * Initialize a stdio xdr stream.
 * Sets the xdr stream handle xdrs for use on the stream file.
 * Operation flag is set to op.
 */
LIBINFO_EXPORT
void
xdrstdio_create(xdrs, file, op)
	XDR *xdrs;
	FILE *file;
	enum xdr_op op;
{
	xdrs->x_op = op;
	xdrs->x_ops = &xdrstdio_ops;
	xdrs->x_private = file;
	xdrs->x_handy = 0;
	xdrs->x_base = 0;
}

/*
 * Destroy a stdio xdr stream.
 * Cleans up the xdr stream handle xdrs previously set up by xdrstdio_create.
 */
static void
xdrstdio_destroy(xdrs)
	XDR *xdrs;
{
	(void)fflush((FILE *)xdrs->x_private);
	/* XXX: should we close the file ?? */
}

static bool_t
xdrstdio_getlong(xdrs, lp)
	XDR *xdrs;
#ifdef __LP64__
	int *lp;
#else
	long *lp;
#endif
{
	u_int32_t temp;

	if (fread(&temp, sizeof(int32_t), 1, (FILE *)xdrs->x_private) != 1)
		return (FALSE);
	*lp = ntohl(temp);
	return (TRUE);
}

static bool_t
xdrstdio_putlong(xdrs, lp)
	XDR *xdrs;
#ifdef __LP64__
	const int *lp;
#else
	const long *lp;
#endif
{
	int32_t mycopy = htonl(*lp);

	if (fwrite(&mycopy, sizeof(int32_t), 1, (FILE *)xdrs->x_private) != 1) return (FALSE);
	return (TRUE);
}

static bool_t
xdrstdio_getbytes(xdrs, addr, len)
	XDR *xdrs;
	char *addr;
	u_int len;
{
	size_t flen = len;

	if ((len != 0) && (fread(addr, flen, 1, (FILE *)xdrs->x_private) != 1)) return (FALSE);
	return (TRUE);
}

static bool_t
xdrstdio_putbytes(xdrs, addr, len)
	XDR *xdrs;
	const char *addr;
	u_int len;
{
	size_t flen = len;

	if ((len != 0) && (fwrite(addr, flen, 1, (FILE *)xdrs->x_private) != 1)) return (FALSE);
	return (TRUE);
}

/* This only works if file offsets are <= UINT32_MAX */
static u_int
xdrstdio_getpos(xdrs)
	XDR *xdrs;
{
	long offset;
	u_int val;

	offset = ftell((FILE *)xdrs->x_private);
#ifdef __LP64__
	if (offset > UINT32_MAX) return -1;
#endif
	val = offset;
	return val;
}

/* This only works if file offsets are <= UINT32_MAX */
static bool_t
xdrstdio_setpos(xdrs, pos) 
	XDR *xdrs;
	u_int pos;
{ 
	long offset;

	offset = pos;
	return ((fseek((FILE *)xdrs->x_private, offset, 0) < 0) ? FALSE : TRUE);
}

/* ARGSUSED */
static int32_t *
xdrstdio_inline(xdrs, len)
	XDR *xdrs;
	u_int len;
{

	/*
	 * Must do some work to implement this: must insure
	 * enough data in the underlying stdio buffer,
	 * that the buffer is aligned so that we can indirect through a
	 * long *, and stuff this pointer in xdrs->x_buf.  Doing
	 * a fread or fwrite to a scratch buffer would defeat
	 * most of the gains to be had here and require storage
	 * management on this buffer, so we don't do this.
	 */
	return (NULL);
}
