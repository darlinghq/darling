/*	$KAME: if_nametoindex.c,v 1.6 2000/11/24 08:18:54 itojun Exp $	*/

/*-
 * Copyright (c) 1997, 2000
 *	Berkeley Software Design, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED BY Berkeley Software Design, Inc. ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Berkeley Software Design, Inc. BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	BSDI Id: if_nametoindex.c,v 2.3 2000/04/17 22:38:05 dab Exp
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
 * From RFC 2553:
 *
 * 4.1 Name-to-Index
 *
 *
 *    The first function maps an interface name into its corresponding
 *    index.
 *
 *       #include <net/if.h>
 *
 *       unsigned int  if_nametoindex(const char *ifname);
 *
 *    If the specified interface name does not exist, the return value is
 *    0, and errno is set to ENXIO.  If there was a system error (such as
 *    running out of memory), the return value is 0 and errno is set to the
 *    proper value (e.g., ENOMEM).
 */

unsigned int
if_nametoindex(const char *ifname)
{
	struct ifaddrs *ifaddrs, *ifa;
	unsigned int ni;

	if (getifaddrs(&ifaddrs) < 0)
		return(0);

	ni = 0;

	for (ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr &&
		    ifa->ifa_addr->sa_family == AF_LINK &&
		    strcmp(ifa->ifa_name, ifname) == 0) {
			ni = ((struct sockaddr_dl*)ifa->ifa_addr)->sdl_index;
			break;
		}
	}

	freeifaddrs(ifaddrs);
	if (!ni)
		errno = ENXIO;
	return(ni);
}
