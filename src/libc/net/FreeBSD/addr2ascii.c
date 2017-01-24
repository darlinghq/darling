/*
 * Copyright 1996 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that both the above copyright notice and this
 * permission notice appear in all copies, that both the above
 * copyright notice and this permission notice appear in all
 * supporting documentation, and that the name of M.I.T. not be used
 * in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  M.I.T. makes
 * no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 * 
 * THIS SOFTWARE IS PROVIDED BY M.I.T. ``AS IS''.  M.I.T. DISCLAIMS
 * ALL EXPRESS OR IMPLIED WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT
 * SHALL M.I.T. BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$ANA: addr2ascii.c,v 1.1 1996/06/13 18:41:46 wollman Exp $
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/net/addr2ascii.c,v 1.2 2002/03/22 21:52:28 obrien Exp $");

#include <sys/types.h>
#include <sys/socket.h>

#include <errno.h>
#include <string.h>

#include <net/if_dl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>

/*-
 * Convert a network address from binary to printable numeric format.
 * This API is copied from INRIA's IPv6 implementation, but it is a
 * bit bogus in two ways:
 *
 *	1) There is no value in passing both an address family and
 *	   an address length; either one should imply the other,
 *	   or we should be passing sockaddrs instead.
 *	2) There should by contrast be /added/ a length for the buffer
 *	   that we pass in, so that programmers are spared the need to
 *	   manually calculate (read: ``guess'') the maximum length.
 *
 * Flash: the API is also the same in the NRL implementation, and seems to
 * be some sort of standard, so we appear to be stuck with both the bad
 * naming and the poor choice of arguments.
 */
char *
addr2ascii(int af, const void *addrp, int len, char *buf)
{
	if (buf == NULL) {
		static char *staticbuf = NULL;
		
		if (staticbuf == NULL) {
			staticbuf = malloc(64); // 64 for AF_LINK > 16 for AF_INET
			if (staticbuf == NULL) {
				return NULL;
			}
		}
		
		buf = staticbuf;
	}

	switch(af) {
	case AF_INET:
		if (len != sizeof(struct in_addr)) {
			errno = ENAMETOOLONG;
			return 0;
		}
		strcpy(buf, inet_ntoa(*(const struct in_addr *)addrp));
		break;

	case AF_LINK:
		if (len != sizeof(struct sockaddr_dl)) {
			errno = ENAMETOOLONG;
			return 0;
		}
		strcpy(buf, link_ntoa((const struct sockaddr_dl *)addrp));
		break;
			
	default:
		errno = EPROTONOSUPPORT;
		return 0;
	}
	return buf;
}
