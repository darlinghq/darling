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
/*static char *sccsid = "from: @(#)get_myaddress.c 1.4 87/08/11 Copyr 1984 Sun Micro";*/
/*static char *sccsid = "from: @(#)get_myaddress.c	2.1 88/07/29 4.0 RPCSRC";*/
static char *rcsid = "$Id: get_myaddress.c,v 1.4 2002/02/19 20:36:23 epeyton Exp $";
#endif

/*
 * get_myaddress.c
 *
 * Get client's IP address via ioctl.  This avoids using the yellowpages.
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * Re-worked for 4.4BSD by Marc Majka at Apple.
 */

#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/pmap_prot.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int
get_myaddress(struct sockaddr_in *addr)
{
	struct ifconf ifc;
	struct ifreq *ifr;
	char buf[BUFSIZ];
	int offset, x, addrlen, s;

	bzero(addr, sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;

	s = socket(AF_INET, SOCK_DGRAM, 0);

	if (s < 0)
	{
		addr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		return 0;
	}
	
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;

	if (ioctl(s, SIOCGIFCONF, (char *)&ifc) < 0)
	{
		addr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		close(s);
		return 0;
	}
	
	addrlen = sizeof(struct ifreq) - IFNAMSIZ;
	offset = 0;

	while (offset <= ifc.ifc_len)
	{
		ifr = (struct ifreq *)(ifc.ifc_buf + offset);
		offset += IFNAMSIZ;
		if (ifr->ifr_addr.sa_len > addrlen) offset += ifr->ifr_addr.sa_len;
		else offset += addrlen;

		if (ifr->ifr_addr.sa_family != AF_INET) continue;
		if (ioctl(s, SIOCGIFFLAGS, ifr) < 0) continue;

		x = ((struct sockaddr_in *)&(ifr->ifr_addr))->sin_addr.s_addr;
		if
		(
			(ifr->ifr_flags & IFF_UP) &&
			(!(ifr->ifr_flags & IFF_LOOPBACK)) &&
			(x != 0) &&
			(x != -1)
		)
		{
			addr->sin_addr.s_addr = x;
			close(s);
			return 0;
		}
	}

	close(s);
	addr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	return 0;
}
