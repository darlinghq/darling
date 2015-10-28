/*
 * Copyright (c) 1996,1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#ifndef __APPLE__
#ifndef lint
static const char rcsid[] = "$Id: ns_netint.c,v 1.1 2006/03/01 19:01:37 majka Exp $";
#endif
#endif

/* Import. */

#ifndef __APPLE__
#include "port_before.h"
#endif

#include <arpa/nameser.h>

#ifndef __APPLE__
#include "port_after.h"
#endif

/* Public. */

u_int
ns_get16(const u_char *src) {
	u_int dst;

	NS_GET16(dst, src);
	return (dst);
}

u_long
ns_get32(const u_char *src) {
	u_long dst;

	NS_GET32(dst, src);
	return (dst);
}

void
ns_put16(u_int src, u_char *dst) {
	NS_PUT16(src, dst);
}

void
ns_put32(u_long src, u_char *dst) {
	NS_PUT32(src, dst);
}
