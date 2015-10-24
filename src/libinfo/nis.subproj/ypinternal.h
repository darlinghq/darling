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
/*	$OpenBSD: ypinternal.h,v 1.2 1996/09/15 09:32:00 tholo Exp $	 */

/*
 * Copyright (c) 1992, 1993, 1996 Theo de Raadt <deraadt@theos.com>
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

/*
 * XXX  We have to define these here due to clashes between
 * yp_prot.h and yp.h.
 */
struct dom_binding {
	struct dom_binding *dom_pnext;
	char dom_domain[YPMAXDOMAIN + 1];
	struct sockaddr_in dom_server_addr;
	unsigned short dom_server_port;
	int dom_socket;
	CLIENT *dom_client;
	unsigned short dom_local_port;
	long dom_vers;
};

#define BINDINGDIR	"/var/yp/binding"
#define YPBINDLOCK	"/var/run/ypbind.lock"

extern struct dom_binding *_ypbindlist;
extern char _yp_domain[MAXHOSTNAMELEN];
extern int _yplib_timeout;

void _yp_unbind __P((struct dom_binding *));
int _yp_check __P((char **));

/* allows callers to cancel yp_bind */
#define ThreadStateExitRequested 4

/* used to tell _yp_dobind to use UDP or TCP */
#define YP_BIND_UDP -1
#define YP_BIND_TCP -2


#ifdef YPMATCHCACHE

static bool_t ypmatch_add __P((const char *, const char *,
    unsigned int, char *, unsigned int));
static bool_t ypmatch_find __P((const char *, const char *,
    unsigned int, char **, unsigned int *));

static struct ypmatch_ent {
	struct ypmatch_ent 	*next;
	char     		*map, *key;
	char           		*val;
	int             	 keylen, vallen;
	time_t          	 expire_t;
} *ypmc;
extern int _yplib_cache;

#endif
