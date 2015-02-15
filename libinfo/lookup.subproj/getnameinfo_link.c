/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * Copyright (c) 2000 Ben Harris.
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
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdint.h>
#include <stdio.h>
#include <netdb.h>
#include <net/if_types.h>
#include <net/if_dl.h>

static int
hexname(const uint8_t *cp, size_t len, char *host, size_t hostlen)
{
	int i, n;
	char *outp = host;

	*outp = '\0';
	for (i = 0; i < len; i++)
	{
		n = snprintf(outp, hostlen, "%s%02x", i ? ":" : "", cp[i]);
		if ((n < 0) || (n >= hostlen))
		{
			*host = '\0';
			return EAI_MEMORY;
		}

		outp += n;
		hostlen -= n;
	}

	return 0;
}

/*
 * getnameinfo_link():
 * Format a link-layer address into a printable format, paying attention to
 * the interface type.
 */
__private_extern__ int
getnameinfo_link(const struct sockaddr *sa, socklen_t salen, char *host, size_t hostlen, char *serv, size_t servlen, int flags)
{
	const struct sockaddr_dl *sdl = (const struct sockaddr_dl *)(const void *)sa;
	int n;

	if (serv != NULL && servlen > 0) *serv = '\0';

	if ((sdl->sdl_nlen == 0) && (sdl->sdl_alen == 0) && (sdl->sdl_slen == 0))
	{
		n = snprintf(host, hostlen, "link#%d", sdl->sdl_index);
		if (n > hostlen)
		{
			*host = '\0';
			return EAI_MEMORY;
		}

		return 0;
	}

	switch (sdl->sdl_type)
	{
			/*
			 * The following have zero-length addresses.
			 * IFT_ATM      (net/if_atmsubr.c)
			 * IFT_FAITH    (net/if_faith.c)
			 * IFT_GIF      (net/if_gif.c)
			 * IFT_LOOP     (net/if_loop.c)
			 * IFT_PPP      (net/if_ppp.c, net/if_spppsubr.c)
			 * IFT_SLIP     (net/if_sl.c, net/if_strip.c)
			 * IFT_STF      (net/if_stf.c)
			 * IFT_L2VLAN   (net/if_vlan.c)
			 * IFT_BRIDGE (net/if_bridge.h>
			 */
			/*
			 * The following use IPv4 addresses as link-layer addresses:
			 * IFT_OTHER    (net/if_gre.c)
			 * IFT_OTHER    (netinet/ip_ipip.c)
			 */
			/* default below is believed correct for all these. */
		case IFT_ARCNET:
		case IFT_ETHER:
		case IFT_FDDI:
		case IFT_HIPPI:
		case IFT_ISO88025:
		default:
			return hexname((uint8_t *)LLADDR(sdl), (size_t)sdl->sdl_alen, host, hostlen);
	}

	/* NOTREACHED */
	return EAI_FAIL;
}
