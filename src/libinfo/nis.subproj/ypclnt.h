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
 * Copyright (c) 1992, 1993 Theo de Raadt <deraadt@fsa.ca>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Theo de Raadt.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _RPCSVC_YPCLNT_H_
#define _RPCSVC_YPCLNT_H_

#include <sys/cdefs.h>

#define YPERR_BADARGS	1		/* args to function are bad */
#define YPERR_RPC	2		/* RPC failure */
#define YPERR_DOMAIN	3		/* can't bind to a server for domain */
#define YPERR_MAP	4		/* no such map in server's domain */
#define YPERR_KEY	5		/* no such key in map */
#define YPERR_YPERR	6		/* some internal YP server or client error */
#define YPERR_RESRC	7		/* local resource allocation failure */
#define YPERR_NOMORE	8		/* no more records in map database */
#define YPERR_PMAP	9		/* can't communicate with portmapper */
#define YPERR_YPBIND	10		/* can't communicate with ypbind */
#define YPERR_YPSERV	11		/* can't communicate with ypserv */
#define YPERR_NODOM	12		/* local domain name not set */
#define YPERR_BADDB	13		/* YP data base is bad */
#define YPERR_VERS	14		/* YP version mismatch */
#define YPERR_ACCESS	15		/* access violation */
#define YPERR_BUSY	16		/* database is busy */

/*
 * Types of update operations
 */
#define YPOP_CHANGE	1		/* change, do not add */
#define YPOP_INSERT	2		/* add, do not change */
#define YPOP_DELETE	3		/* delete this entry */
#define YPOP_STORE 	4		/* add, or change */
 
struct ypall_callback {
	/* return non-0 to stop getting called */
	int (*foreach) __P((unsigned long, char *, int, char *, int, void *));
	char *data;		/* opaque pointer for use of callback fn */
};

__BEGIN_DECLS
int	yp_bind		__P((const char *));
struct dom_binding;
int	_yp_dobind	__P((const char *, struct dom_binding **));
void	yp_unbind	__P((const char *));
int	yp_get_default_domain __P((char **));
int	yp_match 	__P((const char *, const char *, const char *,
			     int , char **, int *));
int	yp_first 	__P((const char *, const char *, char **, int *,
			     char **, int *));
int	yp_next		__P((const char *, const char *, const char *,
			     int, char **, int *, char **, int *));
int	yp_master	__P((const char *, const char *, char **));
int	yp_order	__P((const char *, const char *, int *));
int	yp_all		__P((const char *, const char *,
			     struct ypall_callback *));
char *	yperr_string	__P((int));
int	ypprot_err	__P((unsigned int));
struct ypmaplist;
int	yp_maplist	__P((const char *, struct ypmaplist **));
__END_DECLS

#endif /* _RPCSVC_YPCLNT_H_ */
