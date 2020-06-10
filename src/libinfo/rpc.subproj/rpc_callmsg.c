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
/*static char *sccsid = "from: @(#)rpc_callmsg.c 1.4 87/08/11 Copyr 1984 Sun Micro";*/
/*static char *sccsid = "from: @(#)rpc_callmsg.c	2.1 88/07/29 4.0 RPCSRC";*/
static char *rcsid = "$Id: rpc_callmsg.c,v 1.4 2003/06/23 17:24:59 majka Exp $";
#endif

/*
 * rpc_callmsg.c
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 */

#include "libinfo_common.h"

#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include <rpc/rpc.h>

extern bool_t		xdr_opaque_auth();

/*
 * XDR a call message
 */
LIBINFO_EXPORT
bool_t
xdr_callmsg(xdrs, cmsg)
	register XDR *xdrs;
	register struct rpc_msg *cmsg;
{
#ifdef __LP64__
	int *buf;
#else
	register long *buf;
#endif
	register struct opaque_auth *oa;

	if (xdrs->x_op == XDR_ENCODE) {
		if (cmsg->rm_call.cb_cred.oa_length > MAX_AUTH_BYTES) {
			return (FALSE);
		}
		if (cmsg->rm_call.cb_verf.oa_length > MAX_AUTH_BYTES) {
			return (FALSE);
		}
#ifdef __LP64__
		buf = (int *)XDR_INLINE(xdrs, 8 * BYTES_PER_XDR_UNIT
								 + RNDUP(cmsg->rm_call.cb_cred.oa_length)
								 + 2 * BYTES_PER_XDR_UNIT
								 + RNDUP(cmsg->rm_call.cb_verf.oa_length));
#else
		buf = (long *)XDR_INLINE(xdrs, 8 * BYTES_PER_XDR_UNIT
								 + RNDUP(cmsg->rm_call.cb_cred.oa_length)
								 + 2 * BYTES_PER_XDR_UNIT
								 + RNDUP(cmsg->rm_call.cb_verf.oa_length));
#endif
		if (buf != NULL) {
			IXDR_PUT_LONG(buf, cmsg->rm_xid);
			IXDR_PUT_ENUM(buf, cmsg->rm_direction);
			if (cmsg->rm_direction != CALL) {
				return (FALSE);
			}
			IXDR_PUT_LONG(buf, cmsg->rm_call.cb_rpcvers);
			if (cmsg->rm_call.cb_rpcvers != RPC_MSG_VERSION) {
				return (FALSE);
			}
			IXDR_PUT_LONG(buf, cmsg->rm_call.cb_prog);
			IXDR_PUT_LONG(buf, cmsg->rm_call.cb_vers);
			IXDR_PUT_LONG(buf, cmsg->rm_call.cb_proc);
			oa = &cmsg->rm_call.cb_cred;
			IXDR_PUT_ENUM(buf, oa->oa_flavor);
			IXDR_PUT_LONG(buf, oa->oa_length);
			if (oa->oa_length) {
				bcopy(oa->oa_base, (caddr_t)buf, oa->oa_length);
#ifdef __LP64__
				buf += RNDUP(oa->oa_length) / sizeof (int);
#else
				buf += RNDUP(oa->oa_length) / sizeof (long);
#endif
			}
			oa = &cmsg->rm_call.cb_verf;
			IXDR_PUT_ENUM(buf, oa->oa_flavor);
			IXDR_PUT_LONG(buf, oa->oa_length);
			if (oa->oa_length) {
				bcopy(oa->oa_base, (caddr_t)buf, oa->oa_length);
				/* no real need....
				* N.B. Fix this for __LP64__ if it is uncommented *
				buf += RNDUP(oa->oa_length) / sizeof (long);
				*/
			}
			return (TRUE);
		}
	}
	if (xdrs->x_op == XDR_DECODE) {
#ifdef __LP64__
		buf = (int *)XDR_INLINE(xdrs, 8 * BYTES_PER_XDR_UNIT);
#else
		buf = (long *)XDR_INLINE(xdrs, 8 * BYTES_PER_XDR_UNIT);
#endif
		if (buf != NULL) {
			cmsg->rm_xid = IXDR_GET_LONG(buf);
			cmsg->rm_direction = IXDR_GET_ENUM(buf, enum msg_type);
			if (cmsg->rm_direction != CALL) {
				return (FALSE);
			}
			cmsg->rm_call.cb_rpcvers = IXDR_GET_LONG(buf);
			if (cmsg->rm_call.cb_rpcvers != RPC_MSG_VERSION) {
				return (FALSE);
			}
			cmsg->rm_call.cb_prog = IXDR_GET_LONG(buf);
			cmsg->rm_call.cb_vers = IXDR_GET_LONG(buf);
			cmsg->rm_call.cb_proc = IXDR_GET_LONG(buf);
			oa = &cmsg->rm_call.cb_cred;
			oa->oa_flavor = IXDR_GET_ENUM(buf, enum_t);
			oa->oa_length = IXDR_GET_LONG(buf);
			if (oa->oa_length) {
				if (oa->oa_length > MAX_AUTH_BYTES) {
					return (FALSE);
				}
				if (oa->oa_base == NULL) {
					oa->oa_base = (caddr_t)
						mem_alloc(oa->oa_length);
				}
#ifdef __LP64__
				buf = (int *)XDR_INLINE(xdrs, RNDUP(oa->oa_length));
#else
				buf = (long *)XDR_INLINE(xdrs, RNDUP(oa->oa_length));
#endif
				if (buf == NULL) {
					if (xdr_opaque(xdrs, oa->oa_base,
					    oa->oa_length) == FALSE) {
						return (FALSE);
					}
				} else {
					bcopy((caddr_t)buf, oa->oa_base,
					    oa->oa_length);
					/* no real need....
					* N.B. Fix this for __LP64__ if it is uncommented *
					buf += RNDUP(oa->oa_length) / sizeof (long);
					*/
				}
			}
			oa = &cmsg->rm_call.cb_verf;
#ifdef __LP64__
			buf = (int *)XDR_INLINE(xdrs, 2 * BYTES_PER_XDR_UNIT);
#else
			buf = (long *)XDR_INLINE(xdrs, 2 * BYTES_PER_XDR_UNIT);
#endif
			if (buf == NULL) {
				if (xdr_enum(xdrs, &oa->oa_flavor) == FALSE ||
				    xdr_u_int(xdrs, &oa->oa_length) == FALSE) {
					return (FALSE);
				}
			} else {
				oa->oa_flavor = IXDR_GET_ENUM(buf, enum_t);
				oa->oa_length = IXDR_GET_LONG(buf);
			}
			if (oa->oa_length) {
				if (oa->oa_length > MAX_AUTH_BYTES) {
					return (FALSE);
				}
				if (oa->oa_base == NULL) {
					oa->oa_base = (caddr_t)
						mem_alloc(oa->oa_length);
				}
#ifdef __LP64__
				buf = (int *)XDR_INLINE(xdrs, RNDUP(oa->oa_length));
#else
				buf = (long *)XDR_INLINE(xdrs, RNDUP(oa->oa_length));
#endif
				if (buf == NULL) {
					if (xdr_opaque(xdrs, oa->oa_base,
					    oa->oa_length) == FALSE) {
						return (FALSE);
					}
				} else {
					bcopy((caddr_t)buf, oa->oa_base,
					    oa->oa_length);
					/* no real need...
					* N.B. Fix this for __LP64__ if it is uncommented *
					buf += RNDUP(oa->oa_length) / sizeof (long);
					*/
				}
			}
			return (TRUE);
		}
	}
	if (
	    xdr_u_long(xdrs, &(cmsg->rm_xid)) &&
	    xdr_enum(xdrs, (enum_t *)&(cmsg->rm_direction)) &&
	    (cmsg->rm_direction == CALL) &&
	    xdr_u_long(xdrs, &(cmsg->rm_call.cb_rpcvers)) &&
	    (cmsg->rm_call.cb_rpcvers == RPC_MSG_VERSION) &&
	    xdr_u_long(xdrs, &(cmsg->rm_call.cb_prog)) &&
	    xdr_u_long(xdrs, &(cmsg->rm_call.cb_vers)) &&
	    xdr_u_long(xdrs, &(cmsg->rm_call.cb_proc)) &&
	    xdr_opaque_auth(xdrs, &(cmsg->rm_call.cb_cred)) )
	    return (xdr_opaque_auth(xdrs, &(cmsg->rm_call.cb_verf)));
	return (FALSE);
}

