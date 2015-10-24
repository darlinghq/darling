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

/*	$NetBSD: xdr.c,v 1.22 2000/07/06 03:10:35 christos Exp $	*/

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
static char *sccsid = "@(#)xdr.c 1.35 87/08/12";
static char *sccsid = "@(#)xdr.c	2.1 88/07/29 4.0 RPCSRC";
#endif
#include <sys/cdefs.h>

/*
 * xdr.c, Generic XDR routines implementation.
 *
 * Copyright (C) 1986, Sun Microsystems, Inc.
 *
 * These are the "generic" xdr routines used to serialize and de-serialize
 * most common data items.  See xdr.h for more info on the interface to
 * xdr.
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rpc/types.h>
#include <rpc/xdr.h>

#ifdef __LP64__
#define xdrlong_t int
#else
#define xdrlong_t long
#endif

typedef quad_t          longlong_t;     /* ANSI long long type */
typedef u_quad_t        u_longlong_t;   /* ANSI unsigned long long type */

/*
 * constants specific to the xdr "protocol"
 */
#ifdef __LP64__
#define XDR_FALSE	((int) 0)
#define XDR_TRUE	((int) 1)
#else
#define XDR_FALSE	((long) 0)
#define XDR_TRUE	((long) 1)
#endif
#define LASTUNSIGNED	((u_int) 0-1)

/*
 * for unit alignment
 */
static const char xdr_zero[BYTES_PER_XDR_UNIT] = { 0, 0, 0, 0 };

/*
 * Free a data structure using XDR
 * Not a filter, but a convenient utility nonetheless
 */
void
xdr_free(proc, objp)
	xdrproc_t proc;
	void *objp;
{
	XDR x;
	
	x.x_op = XDR_FREE;
	(*proc)(&x, objp, 0);
}

/*
 * XDR nothing
 */
bool_t
xdr_void(void)
{

	return (TRUE);
}


/*
 * XDR integers
 */
bool_t
xdr_int(xdrs, ip)
	XDR *xdrs;
	int *ip;
{
	xdrlong_t l;

	switch (xdrs->x_op) {

	case XDR_ENCODE:
		l = *ip;
		return (XDR_PUTLONG(xdrs, (const xdrlong_t *)&l));

	case XDR_DECODE:
		if (!XDR_GETLONG(xdrs, &l)) {
			return (FALSE);
		}
		*ip = l;
		return (TRUE);

	case XDR_FREE:
		return (TRUE);
	}
	/* NOTREACHED */
	return (FALSE);
}

/*
 * XDR unsigned integers
 */
bool_t
xdr_u_int(xdrs, up)
	XDR *xdrs;
	u_int *up;
{
	xdrlong_t l;

	switch (xdrs->x_op) {

	case XDR_ENCODE:
		l = *up;
		return (XDR_PUTLONG(xdrs, (const xdrlong_t *)&l));

	case XDR_DECODE:
		if (!XDR_GETLONG(xdrs, &l)) {
			return (FALSE);
		}
		*up = l;
		return (TRUE);

	case XDR_FREE:
		return (TRUE);
	}
	/* NOTREACHED */
	return (FALSE);
}


/*
 * XDR long integers
 * same as xdr_u_long - open coded to save a proc call!
 */
bool_t
xdr_long(xdrs, lp)
	XDR *xdrs;
#ifdef __LP64__
	int *lp;
#else
	long *lp;
#endif
{
	switch (xdrs->x_op) {
	case XDR_ENCODE:
		return (XDR_PUTLONG(xdrs, lp));
	case XDR_DECODE:
		return (XDR_GETLONG(xdrs, lp));
	case XDR_FREE:
		return (TRUE);
	}
	/* NOTREACHED */
	return (FALSE);
}

/*
 * XDR unsigned long integers
 * same as xdr_long - open coded to save a proc call!
 */
bool_t
xdr_u_long(xdrs, ulp)
	XDR *xdrs;
#ifdef __LP64__
	unsigned int *ulp;
#else
	u_long *ulp;
#endif
{	
	switch (xdrs->x_op) {
	case XDR_ENCODE:
		return (XDR_PUTLONG(xdrs, (const xdrlong_t *)ulp));
	case XDR_DECODE:
		return (XDR_GETLONG(xdrs, (xdrlong_t *)ulp));
	case XDR_FREE:
		return (TRUE);
	}
	/* NOTREACHED */
	return (FALSE);
}


/*
 * XDR 32-bit integers
 * same as xdr_u_int32_t - open coded to save a proc call!
 */
bool_t
xdr_int32_t(xdrs, int32_p)
	XDR *xdrs;
	int32_t *int32_p;
{
	xdrlong_t l;

	switch (xdrs->x_op) {

	case XDR_ENCODE:
		l = *int32_p;
		return (XDR_PUTLONG(xdrs, (const xdrlong_t *)&l));

	case XDR_DECODE:
		if (!XDR_GETLONG(xdrs, &l)) {
			return (FALSE);
		}
		*int32_p = l;
		return (TRUE);

	case XDR_FREE:
		return (TRUE);
	}
	/* NOTREACHED */
	return (FALSE);
}

/*
 * XDR unsigned 32-bit integers
 * same as xdr_int32_t - open coded to save a proc call!
 */
bool_t
xdr_u_int32_t(xdrs, u_int32_p)
	XDR *xdrs;
	u_int32_t *u_int32_p;
{
	u_int32_t l;

	switch (xdrs->x_op) {

	case XDR_ENCODE:
		l = *u_int32_p;
		return (XDR_PUTLONG(xdrs, (xdrlong_t *)&l));

	case XDR_DECODE:
		if (!XDR_GETLONG(xdrs, (xdrlong_t *)&l)) return (FALSE);
		*u_int32_p = l;
		return (TRUE);

	case XDR_FREE:
		return (TRUE);
	}
	/* NOTREACHED */
	return (FALSE);
}


/*
 * XDR short integers
 */
bool_t
xdr_short(xdrs, sp)
	XDR *xdrs;
	short *sp;
{
	xdrlong_t l;

	switch (xdrs->x_op) {

	case XDR_ENCODE:
		l = *sp;
		return (XDR_PUTLONG(xdrs, (const xdrlong_t *)&l));

	case XDR_DECODE:
		if (!XDR_GETLONG(xdrs, &l)) {
			return (FALSE);
		}
		*sp = l;
		return (TRUE);

	case XDR_FREE:
		return (TRUE);
	}
	/* NOTREACHED */
	return (FALSE);
}

/*
 * XDR unsigned short integers
 */
bool_t
xdr_u_short(xdrs, usp)
	XDR *xdrs;
	u_short *usp;
{
	xdrlong_t l;

	switch (xdrs->x_op) {

	case XDR_ENCODE:
		l = *usp;
		return (XDR_PUTLONG(xdrs, (const xdrlong_t *)&l));

	case XDR_DECODE:
		if (!XDR_GETLONG(xdrs, &l)) {
			return (FALSE);
		}
		*usp = l;
		return (TRUE);

	case XDR_FREE:
		return (TRUE);
	}
	/* NOTREACHED */
	return (FALSE);
}


/*
 * XDR 16-bit integers
 */
bool_t
xdr_int16_t(xdrs, int16_p)
	XDR *xdrs;
	int16_t *int16_p;
{
	xdrlong_t l;

	switch (xdrs->x_op) {

	case XDR_ENCODE:
		l = *int16_p;
		return (XDR_PUTLONG(xdrs, (const xdrlong_t *)&l));

	case XDR_DECODE:
		if (!XDR_GETLONG(xdrs, &l)) {
			return (FALSE);
		}
		*int16_p = l;
		return (TRUE);

	case XDR_FREE:
		return (TRUE);
	}
	/* NOTREACHED */
	return (FALSE);
}

/*
 * XDR unsigned 16-bit integers
 */
bool_t
xdr_u_int16_t(xdrs, u_int16_p)
	XDR *xdrs;
	u_int16_t *u_int16_p;
{
	xdrlong_t l;

	switch (xdrs->x_op) {

	case XDR_ENCODE:
		l = *u_int16_p;
		return (XDR_PUTLONG(xdrs, (const xdrlong_t *)&l));

	case XDR_DECODE:
		if (!XDR_GETLONG(xdrs, &l)) {
			return (FALSE);
		}
		*u_int16_p = l;
		return (TRUE);

	case XDR_FREE:
		return (TRUE);
	}
	/* NOTREACHED */
	return (FALSE);
}


/*
 * XDR a char
 */
bool_t
xdr_char(xdrs, cp)
	XDR *xdrs;
	char *cp;
{
	int i;

	i = (*cp);
	if (!xdr_int(xdrs, &i)) {
		return (FALSE);
	}
	*cp = i;
	return (TRUE);
}

/*
 * XDR an unsigned char
 */
bool_t
xdr_u_char(xdrs, cp)
	XDR *xdrs;
	u_char *cp;
{
	u_int32_t u;

	u = (*cp);
	if (!xdr_u_int(xdrs, &u)) {
		return (FALSE);
	}
	*cp = u;
	return (TRUE);
}

/*
 * XDR booleans
 */
bool_t
xdr_bool(xdrs, bp)
	XDR *xdrs;
	bool_t *bp;
{
	xdrlong_t lb;

	switch (xdrs->x_op) {

	case XDR_ENCODE:
		lb = *bp ? XDR_TRUE : XDR_FALSE;
		return (XDR_PUTLONG(xdrs, (const xdrlong_t *)&lb));

	case XDR_DECODE:
		if (!XDR_GETLONG(xdrs, &lb)) {
			return (FALSE);
		}
		*bp = (lb == XDR_FALSE) ? FALSE : TRUE;
		return (TRUE);

	case XDR_FREE:
		return (TRUE);
	}
	/* NOTREACHED */
	return (FALSE);
}

/*
 * XDR enumerations
 */
bool_t
xdr_enum(xdrs, ep)
	XDR *xdrs;
	enum_t *ep;
{
	enum sizecheck { SIZEVAL };	/* used to find the size of an enum */

	/*
	 * enums are treated as ints
	 */
	/* LINTED */ if (sizeof (enum sizecheck) == sizeof (int)) {
		return (xdr_long(xdrs, (xdrlong_t *)(void *)ep));
	} else /* LINTED */ if (sizeof (enum sizecheck) == sizeof (int)) {
		return (xdr_int(xdrs, (int *)(void *)ep));
	} else /* LINTED */ if (sizeof (enum sizecheck) == sizeof (short)) {
		return (xdr_short(xdrs, (short *)(void *)ep));
	} else {
		return (FALSE);
	}
}

/*
 * XDR opaque data
 * Allows the specification of a fixed size sequence of opaque bytes.
 * cp points to the opaque object and cnt gives the byte length.
 */
bool_t
xdr_opaque(xdrs, cp, cnt)
	XDR *xdrs;
	caddr_t cp;
	u_int cnt;
{
	u_int32_t rndup;
	u_int8_t crud[BYTES_PER_XDR_UNIT];

	/*
	 * if no data we are done
	 */
	if (cnt == 0)
		return (TRUE);

	/*
	 * round byte count to full xdr units
	 */
	rndup = cnt % BYTES_PER_XDR_UNIT;
	if (rndup > 0)
		rndup = BYTES_PER_XDR_UNIT - rndup;

	if (xdrs->x_op == XDR_DECODE) {
		if (!XDR_GETBYTES(xdrs, cp, cnt)) {
			return (FALSE);
		}
		if (rndup == 0)
			return (TRUE);
		return (XDR_GETBYTES(xdrs, (caddr_t)(void *)crud, rndup));
	}

	if (xdrs->x_op == XDR_ENCODE) {
		if (!XDR_PUTBYTES(xdrs, cp, cnt)) {
			return (FALSE);
		}
		if (rndup == 0)
			return (TRUE);
		return (XDR_PUTBYTES(xdrs, xdr_zero, rndup));
	}

	if (xdrs->x_op == XDR_FREE) {
		return (TRUE);
	}

	return (FALSE);
}

/*
 * XDR counted bytes
 * *cpp is a pointer to the bytes, *sizep is the count.
 * If *cpp is NULL maxsize bytes are allocated
 */
bool_t
xdr_bytes(xdrs, cpp, sizep, maxsize)
	XDR *xdrs;
	char **cpp;
	u_int *sizep;
	u_int maxsize;
{
	char *sp = *cpp;  /* sp is the actual string pointer */
	u_int32_t nodesize;

	/*
	 * first deal with the length since xdr bytes are counted
	 */
	if (! xdr_u_int(xdrs, sizep)) {
		return (FALSE);
	}
	nodesize = *sizep;
	if ((nodesize > maxsize) && (xdrs->x_op != XDR_FREE)) {
		return (FALSE);
	}

	/*
	 * now deal with the actual bytes
	 */
	switch (xdrs->x_op) {

	case XDR_DECODE:
		if (nodesize == 0) {
			return (TRUE);
		}
		if (sp == NULL) {
			*cpp = sp = mem_alloc(nodesize);
		}
		if (sp == NULL) {
			warnx("xdr_bytes: out of memory");
			return (FALSE);
		}
		/* FALLTHROUGH */

	case XDR_ENCODE:
		return (xdr_opaque(xdrs, sp, nodesize));

	case XDR_FREE:
		if (sp != NULL) {
			mem_free(sp, nodesize);
			*cpp = NULL;
		}
		return (TRUE);
	}
	/* NOTREACHED */
	return (FALSE);
}

/*
 * Implemented here due to commonality of the object.
 */
bool_t
xdr_netobj(xdrs, np)
	XDR *xdrs;
	struct netobj *np;
{

	return (xdr_bytes(xdrs, &np->n_bytes, &np->n_len, MAX_NETOBJ_SZ));
}

/*
 * XDR a descriminated union
 * Support routine for discriminated unions.
 * You create an array of xdrdiscrim structures, terminated with
 * an entry with a null procedure pointer.  The routine gets
 * the discriminant value and then searches the array of xdrdiscrims
 * looking for that value.  It calls the procedure given in the xdrdiscrim
 * to handle the discriminant.  If there is no specific routine a default
 * routine may be called.
 * If there is no specific or default routine an error is returned.
 */
bool_t
xdr_union(xdrs, dscmp, unp, choices, dfault)
	XDR *xdrs;
	enum_t *dscmp;		/* enum to decide which arm to work on */
	char *unp;		/* the union itself */
	const struct xdr_discrim *choices;	/* [value, xdr proc] for each arm */
	xdrproc_t dfault;	/* default xdr routine */
{
	enum_t dscm;

	/*
	 * we deal with the discriminator;  it's an enum
	 */
	if (! xdr_enum(xdrs, dscmp)) {
		return (FALSE);
	}
	dscm = *dscmp;

	/*
	 * search choices for a value that matches the discriminator.
	 * if we find one, execute the xdr routine for that value.
	 */
	for (; choices->proc != NULL_xdrproc_t; choices++) {
		if (choices->value == dscm)
			return ((*(choices->proc))(xdrs, unp, 0));
	}

	/*
	 * no match - execute the default xdr routine if there is one
	 */
	return ((dfault == NULL_xdrproc_t) ? FALSE :
	    (*dfault)(xdrs, unp, 0));
}


/*
 * Non-portable xdr primitives.
 * Care should be taken when moving these routines to new architectures.
 */


/*
 * XDR null terminated ASCII strings
 * xdr_string deals with "C strings" - arrays of bytes that are
 * terminated by a NULL character.  The parameter cpp references a
 * pointer to storage; If the pointer is null, then the necessary
 * storage is allocated.  The last parameter is the max allowed length
 * of the string as specified by a protocol.
 */
bool_t
xdr_string(xdrs, cpp, maxsize)
	XDR *xdrs;
	char **cpp;
	u_int maxsize;
{
	char *sp = *cpp;  /* sp is the actual string pointer */
	u_int32_t size;
	u_int32_t nodesize;

	/*
	 * first deal with the length since xdr strings are counted-strings
	 */
	switch (xdrs->x_op) {
	case XDR_FREE:
		if (sp == NULL) {
			return(TRUE);	/* already free */
		}
		/* FALLTHROUGH */
	case XDR_ENCODE:
		size = strlen(sp);
		break;
	case XDR_DECODE:
		break;
	}
	if (! xdr_u_int(xdrs, &size)) {
		return (FALSE);
	}
	if (size > maxsize) {
		return (FALSE);
	}
	nodesize = size + 1;

	/*
	 * now deal with the actual bytes
	 */
	switch (xdrs->x_op) {

	case XDR_DECODE:
		if (nodesize == 0) {
			return (TRUE);
		}
		if (sp == NULL)
			*cpp = sp = mem_alloc(nodesize);
		if (sp == NULL) {
			warnx("xdr_string: out of memory");
			return (FALSE);
		}
		sp[size] = 0;
		/* FALLTHROUGH */

	case XDR_ENCODE:
		return (xdr_opaque(xdrs, sp, size));

	case XDR_FREE:
		mem_free(sp, nodesize);
		*cpp = NULL;
		return (TRUE);
	}
	/* NOTREACHED */
	return (FALSE);
}

/* 
 * Wrapper for xdr_string that can be called directly from 
 * routines like clnt_call
 */
bool_t
xdr_wrapstring(xdrs, cpp)
	XDR *xdrs;
	char **cpp;
{
	return xdr_string(xdrs, cpp, LASTUNSIGNED);
}

/*
 * NOTE: xdr_hyper(), xdr_u_hyper(), xdr_longlong_t(), and xdr_u_longlong_t()
 * are in the "non-portable" section because they require that a `long long'
 * be a 64-bit type.
 *
 *	--thorpej@netbsd.org, November 30, 1999
 */

/*
 * XDR 64-bit integers
 */
bool_t
xdr_int64_t(xdrs, llp)
	XDR *xdrs;
	int64_t *llp;
{
	u_int32_t ul[2];

	switch (xdrs->x_op) {
	case XDR_ENCODE:
		ul[0] = (u_int32_t)((u_int64_t)*llp >> 32) & 0xffffffff;
		ul[1] = (u_int32_t)((u_int64_t)*llp) & 0xffffffff;
		if (XDR_PUTLONG(xdrs, (xdrlong_t *)&ul[0]) == FALSE)
			return (FALSE);
		return (XDR_PUTLONG(xdrs, (xdrlong_t *)&ul[1]));
	case XDR_DECODE:
		if (XDR_GETLONG(xdrs, (xdrlong_t *)&ul[0]) == FALSE)
			return (FALSE);
		if (XDR_GETLONG(xdrs, (xdrlong_t *)&ul[1]) == FALSE)
			return (FALSE);
		*llp = (int64_t)
		    (((u_int64_t)ul[0] << 32) | ((u_int64_t)ul[1]));
		return (TRUE);
	case XDR_FREE:
		return (TRUE);
	}
	/* NOTREACHED */
	return (FALSE);
}


/*
 * XDR unsigned 64-bit integers
 */
bool_t
xdr_u_int64_t(xdrs, ullp)
	XDR *xdrs;
	u_int64_t *ullp;
{
	u_int32_t ul[2];

	switch (xdrs->x_op) {
	case XDR_ENCODE:
		ul[0] = (u_int32_t)(*ullp >> 32) & 0xffffffff;
		ul[1] = (u_int32_t)(*ullp) & 0xffffffff;
		if (XDR_PUTLONG(xdrs, (xdrlong_t *)&ul[0]) == FALSE)
			return (FALSE);
		return (XDR_PUTLONG(xdrs, (xdrlong_t *)&ul[1]));
	case XDR_DECODE:
		if (XDR_GETLONG(xdrs, (xdrlong_t *)&ul[0]) == FALSE)
			return (FALSE);
		if (XDR_GETLONG(xdrs, (xdrlong_t *)&ul[1]) == FALSE)
			return (FALSE);
		*ullp = (u_int64_t)
		    (((u_int64_t)ul[0] << 32) | ((u_int64_t)ul[1]));
		return (TRUE);
	case XDR_FREE:
		return (TRUE);
	}
	/* NOTREACHED */
	return (FALSE);
}


/*
 * XDR hypers
 */
bool_t
xdr_hyper(xdrs, llp)
	XDR *xdrs;
	longlong_t *llp;
{
	/*
	 * Don't bother open-coding this; it's a fair amount of code.  Just
	 * call xdr_int64_t().
	 */
	return (xdr_int64_t(xdrs, (int64_t *)llp));
}


/*
 * XDR unsigned hypers
 */
bool_t
xdr_u_hyper(xdrs, ullp)
	XDR *xdrs;
	u_longlong_t *ullp;
{
	/*
	 * Don't bother open-coding this; it's a fair amount of code.  Just
	 * call xdr_u_int64_t().
	 */
	return (xdr_u_int64_t(xdrs, (u_int64_t *)ullp));
}


/*
 * XDR longlong_t's
 */
bool_t
xdr_longlong_t(xdrs, llp)
	XDR *xdrs;
	longlong_t *llp;
{
	/*
	 * Don't bother open-coding this; it's a fair amount of code.  Just
	 * call xdr_int64_t().
	 */
	return (xdr_int64_t(xdrs, (int64_t *)llp));
}


/*
 * XDR u_longlong_t's
 */
bool_t
xdr_u_longlong_t(xdrs, ullp)
	XDR *xdrs;
	u_longlong_t *ullp;
{

	/*
	 * Don't bother open-coding this; it's a fair amount of code.  Just
	 * call xdr_u_int64_t().
	 */
	return (xdr_u_int64_t(xdrs, (u_int64_t *)ullp));
}
