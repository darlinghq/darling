/*
 * Copyright (c) 1999-2008 Apple Inc. All rights reserved.
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
#if !defined(lint) && defined(SCCSIDS)
static  char sccsid[] = "@(#)ether_addr.c	1.2 88/05/10 4.0NFSSRC; from 1.9 88/02/08 Copyr 1985 Sun Micro";
#endif

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 *
 * All routines necessary to deal with the file /etc/ethers.  The file
 * contains mappings from 48 bit ethernet addresses to their corresponding
 * hosts name.  The addresses have an ascii representation of the form
 * "x:x:x:x:x:x" where x is a hex number between 0x00 and 0xff;  the
 * bytes are always in network order.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>

/*
 * Parses a line from /etc/ethers into its components.  The line has the form
 * 8:0:20:1:17:c8	krypton
 * where the first part is a 48 bit ethernet addrerss and the second is
 * the corresponding hosts name.
 * Returns zero if successful, non-zero otherwise.
 */
int
ether_line(const char *s, struct ether_addr *e, char *hostname)
{
	int i;
	unsigned int t[6];
	
	i = sscanf(s, " %x:%x:%x:%x:%x:%x %s", &t[0], &t[1], &t[2], &t[3], &t[4], &t[5], hostname);
	if (i != 7) return (7 - i);

	for (i = 0; i < 6; i++) e->ether_addr_octet[i] = t[i];
	return 0;
}

/*
 * Converts a 48 bit ethernet number to its string representation.
 */
#define EI(i) (unsigned int)(e->ether_addr_octet[(i)])
char *
ether_ntoa(const struct ether_addr *e)
{
	static char *s;

	if (s == 0)
	{
		s = (char *)malloc(18);
		if (s == 0) return NULL;
	}

	s[0] = 0;
	sprintf(s, "%x:%x:%x:%x:%x:%x", EI(0), EI(1), EI(2), EI(3), EI(4), EI(5));
	return s;
}

/*
 * Converts a ethernet address representation back into its 48 bits.
 */
struct ether_addr *
ether_aton(const char *s)
{
	static struct ether_addr *ep;
	int i;
	unsigned int t[6];
	
	if (ep == 0)
	{
		ep = (struct ether_addr *)calloc(1, sizeof (struct ether_addr));
		if (ep == 0) return NULL;
	}

	i = sscanf(s, " %x:%x:%x:%x:%x:%x", &t[0], &t[1], &t[2], &t[3], &t[4], &t[5]);
	if (i != 6) return NULL;

	for (i = 0; i < 6; i++) ep->ether_addr_octet[i] = t[i];
	return ep;
}
