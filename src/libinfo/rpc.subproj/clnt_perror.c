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
/*static char *sccsid = "from: @(#)clnt_perror.c 1.15 87/10/07 Copyr 1984 Sun Micro";*/
/*static char *sccsid = "from: @(#)clnt_perror.c	2.1 88/07/29 4.0 RPCSRC";*/
static char *rcsid = "$Id: clnt_perror.c,v 1.4 2003/06/23 17:24:59 majka Exp $";
#endif

/*
 * clnt_perror.c
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 */
#include "libinfo_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc/rpc.h>
#include <rpc/types.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>

static char *auth_errmsg();

static char *buf;

static char *
_buf()
{

	if (buf == 0)
		buf = (char *)malloc(256);
	return (buf);
}

/*
 * Print reply error info
 */
LIBINFO_EXPORT
char *
clnt_sperror(rpch, s)
	CLIENT *rpch;
	char *s;
{
	struct rpc_err e;
	void clnt_perrno();
	char *err;
	char *str = _buf();
	char *strstart = str;

	if (str == 0)
		return (0);
	CLNT_GETERR(rpch, &e);

	(void) sprintf(str, "%s: ", s);  
	str += strlen(str);

	(void) strcpy(str, clnt_sperrno(e.re_status));  
	str += strlen(str);

	switch (e.re_status) {
	case RPC_SUCCESS:
	case RPC_CANTENCODEARGS:
	case RPC_CANTDECODERES:
	case RPC_TIMEDOUT:     
	case RPC_PROGUNAVAIL:
	case RPC_PROCUNAVAIL:
	case RPC_CANTDECODEARGS:
	case RPC_SYSTEMERROR:
	case RPC_UNKNOWNHOST:
	case RPC_UNKNOWNPROTO:
	case RPC_PMAPFAILURE:
	case RPC_PROGNOTREGISTERED:
	case RPC_FAILED:
		break;

	case RPC_CANTSEND:
	case RPC_CANTRECV:
		(void) sprintf(str, "; errno = %s",
		    strerror(e.re_errno)); 
		str += strlen(str);
		break;

	case RPC_VERSMISMATCH:
#ifdef __LP64__
		(void) sprintf(str, "; low version = %u, high version = %u", e.re_vers.low, e.re_vers.high);
#else
		(void) sprintf(str, "; low version = %lu, high version = %lu", e.re_vers.low, e.re_vers.high);
#endif
		str += strlen(str);
		break;

	case RPC_AUTHERROR:
		err = auth_errmsg(e.re_why);
		(void) sprintf(str,"; why = ");
		str += strlen(str);
		if (err != NULL) {
			(void) sprintf(str, "%s",err);
		} else {
			(void) sprintf(str,
				"(unknown authentication error - %d)",
				(int) e.re_why);
		}
		str += strlen(str);
		break;

	case RPC_PROGVERSMISMATCH:
#ifdef __LP64__
		(void) sprintf(str, "; low version = %u, high version = %u",  e.re_vers.low, e.re_vers.high);
#else
		(void) sprintf(str, "; low version = %lu, high version = %lu",  e.re_vers.low, e.re_vers.high);
#endif
		str += strlen(str);
		break;

	default:	/* unknown */
#ifdef __LP64__
		(void) sprintf(str, "; s1 = %u, s2 = %u", e.re_lb.s1, e.re_lb.s2);
#else
		(void) sprintf(str, "; s1 = %lu, s2 = %lu", e.re_lb.s1, e.re_lb.s2);
#endif
		str += strlen(str);
		break;
	}
	(void) sprintf(str, "\n");
	return(strstart) ;
}

LIBINFO_EXPORT
void
clnt_perror(rpch, s)
	CLIENT *rpch;
	char *s;
{
	(void) fprintf(stderr,"%s",clnt_sperror(rpch,s));
}


struct rpc_errtab {
	enum clnt_stat status;
	char *message;
};

static struct rpc_errtab  rpc_errlist[] = {
	{ RPC_SUCCESS, 
		"RPC: Success" }, 
	{ RPC_CANTENCODEARGS, 
		"RPC: Can't encode arguments" },
	{ RPC_CANTDECODERES, 
		"RPC: Can't decode result" },
	{ RPC_CANTSEND, 
		"RPC: Unable to send" },
	{ RPC_CANTRECV, 
		"RPC: Unable to receive" },
	{ RPC_TIMEDOUT, 
		"RPC: Timed out" },
	{ RPC_VERSMISMATCH, 
		"RPC: Incompatible versions of RPC" },
	{ RPC_AUTHERROR, 
		"RPC: Authentication error" },
	{ RPC_PROGUNAVAIL, 
		"RPC: Program unavailable" },
	{ RPC_PROGVERSMISMATCH, 
		"RPC: Program/version mismatch" },
	{ RPC_PROCUNAVAIL, 
		"RPC: Procedure unavailable" },
	{ RPC_CANTDECODEARGS, 
		"RPC: Server can't decode arguments" },
	{ RPC_SYSTEMERROR, 
		"RPC: Remote system error" },
	{ RPC_UNKNOWNHOST, 
		"RPC: Unknown host" },
	{ RPC_UNKNOWNPROTO,
		"RPC: Unknown protocol" },
	{ RPC_PMAPFAILURE, 
		"RPC: Port mapper failure" },
	{ RPC_PROGNOTREGISTERED, 
		"RPC: Program not registered"},
	{ RPC_FAILED, 
		"RPC: Failed (unspecified error)"}
};


/*
 * This interface for use by clntrpc
 */
LIBINFO_EXPORT
char *
clnt_sperrno(stat)
	enum clnt_stat stat;
{
	int i;

	for (i = 0; i < sizeof(rpc_errlist)/sizeof(struct rpc_errtab); i++) {
		if (rpc_errlist[i].status == stat) {
			return (rpc_errlist[i].message);
		}
	}
	return ("RPC: (unknown error code)");
}

LIBINFO_EXPORT
void
clnt_perrno(num)
	enum clnt_stat num;
{
	(void) fprintf(stderr,"%s",clnt_sperrno(num));
}


LIBINFO_EXPORT
char *
clnt_spcreateerror(s)
	char *s;
{
	char *str = _buf();

	if (str == 0)
		return(0);
	(void) sprintf(str, "%s: ", s);
	(void) strcat(str, clnt_sperrno(rpc_createerr.cf_stat));
	switch (rpc_createerr.cf_stat) {
	case RPC_PMAPFAILURE:
		(void) strcat(str, " - ");
		(void) strcat(str,
		    clnt_sperrno(rpc_createerr.cf_error.re_status));
		break;

	case RPC_SYSTEMERROR:
		(void) strcat(str, " - ");
		if (rpc_createerr.cf_error.re_errno > 0
		    && rpc_createerr.cf_error.re_errno < sys_nerr)
			(void) strcat(str,
			    strerror(rpc_createerr.cf_error.re_errno));
		else
			(void) sprintf(&str[strlen(str)], "Error %d",
			    rpc_createerr.cf_error.re_errno);
		break;
    default: break;
	}
	(void) strcat(str, "\n");
	return (str);
}

LIBINFO_EXPORT
void
clnt_pcreateerror(s)
	char *s;
{
	(void) fprintf(stderr,"%s",clnt_spcreateerror(s));
}

struct auth_errtab {
	enum auth_stat status;	
	char *message;
};

static struct auth_errtab auth_errlist[] = {
	{ AUTH_OK,
		"Authentication OK" },
	{ AUTH_BADCRED,
		"Invalid client credential" },
	{ AUTH_REJECTEDCRED,
		"Server rejected credential" },
	{ AUTH_BADVERF,
		"Invalid client verifier" },
	{ AUTH_REJECTEDVERF,
		"Server rejected verifier" },
	{ AUTH_TOOWEAK,
		"Client credential too weak" },
	{ AUTH_INVALIDRESP,
		"Invalid server verifier" },
	{ AUTH_FAILED,
		"Failed (unspecified error)" },
};

static char *
auth_errmsg(stat)
	enum auth_stat stat;
{
	int i;

	for (i = 0; i < sizeof(auth_errlist)/sizeof(struct auth_errtab); i++) {
		if (auth_errlist[i].status == stat) {
			return(auth_errlist[i].message);
		}
	}
	return(NULL);
}
