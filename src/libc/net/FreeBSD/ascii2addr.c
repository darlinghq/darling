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
 *	$ANA: ascii2addr.c,v 1.2 1996/06/13 18:46:02 wollman Exp $
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/net/ascii2addr.c,v 1.4 2002/03/22 21:52:28 obrien Exp $");

#include <sys/types.h>
#include <sys/socket.h>

#include <errno.h>
#include <string.h>

#include <net/if_dl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int
ascii2addr(af, ascii, result)
	int af;
	const char *ascii;
	void *result;
{
	struct in_addr *ina;
	char strbuf[4*sizeof("123")]; /* long enough for V4 only */

	switch(af) {
	case AF_INET:
		ina = result;
		strbuf[0] = '\0';
		strncat(strbuf, ascii, (sizeof strbuf)-1);
		if (inet_aton(strbuf, ina))
			return sizeof(struct in_addr);
		errno = EINVAL;
		break;

	case AF_LINK:
		link_addr(ascii, result);
		/* oops... no way to detect failure */
		return sizeof(struct sockaddr_dl);

	default:
		errno = EPROTONOSUPPORT;
		break;
	}

	return -1;
}
