/*
 * Copyright (c) 2008-2011 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Portions Copyright (c) 1996-1999 by Internet Software Consortium.
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
/*
 * Copyright (c) 1988, 1993
 *    The Regents of the University of California.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * Portions Copyright (c) 1993 by Digital Equipment Corporation.
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies, and that
 * the name of Digital Equipment Corporation not be used in advertising or
 * publicity pertaining to distribution of the document or software without
 * specific, written prior permission.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#include "ils.h"
#include "netdb.h"
#include "si_module.h"

#include <assert.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <arpa/nameser_compat.h>
#include <libkern/OSAtomic.h>
#include <netinet/in.h>
#include <ctype.h>
#include <dns_sd.h>
#include <dnsinfo.h>
#include <errno.h>
#include <nameser.h>
#include <notify.h>
#include <pthread.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <time.h>
#include <unistd.h>
#include <asl.h>
#include <dns.h>
#include <dns_util.h>
#include <TargetConditionals.h>
#include <dispatch/dispatch.h>

/* from dns_util.c */
#define DNS_MAX_RECEIVE_SIZE 65536

#define INET_NTOP_AF_INET_OFFSET 4
#define INET_NTOP_AF_INET6_OFFSET 8

#define IPPROTO_UNSPEC 0

#define GOT_DATA 1
#define GOT_ERROR 2
#define SHORT_AAAA_EXTRA 2
#define MEDIUM_AAAA_EXTRA 5
#define LONG_AAAA_EXTRA 10

#define MDNS_DEBUG_FILE "/etc/.mdns_debug"
#define MDNS_DEBUG_STDOUT 0x00000001
#define MDNS_DEBUG_STDERR 0x00000002
#define MDNS_DEBUG_ASL    0x00000004
#define MDNS_DEBUG_OUT    0x00000007
#define MDNS_DEBUG_MORE   0x00000010

static int _mdns_debug = 0;

// mutex protects DNSServiceProcessResult and DNSServiceRefDeallocate
static pthread_mutex_t _mdns_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
	uint16_t priority;
	uint16_t weight;
	uint16_t port;
	uint8_t target[0];
} mdns_rr_srv_t;

typedef struct mdns_srv_t mdns_srv_t;
struct mdns_srv_t {
	si_srv_t srv;
	mdns_srv_t *next;
};

typedef struct {
	struct hostent host;
	int alias_count;
	int addr_count;
} mdns_hostent_t;

typedef struct {
	mdns_hostent_t *h4;
	mdns_hostent_t *h6;
	mdns_srv_t *srv;
	uint64_t ttl;
	uint32_t ifnum;
} mdns_reply_t;

static uint32_t _mdns_generation = 0;
static DNSServiceRef _mdns_sdref;
static DNSServiceRef _mdns_old_sdref;

static void _mdns_hostent_clear(mdns_hostent_t *h);
static void _mdns_reply_clear(mdns_reply_t *r);
static int _mdns_search(const char *name, int class, int type, const char *interface, DNSServiceFlags flags, uint8_t *answer, uint32_t *anslen, mdns_reply_t *reply);

static const char hexchar[] = "0123456789abcdef";

#define BILLION 1000000000

/* length of a reverse DNS IPv6 address query name, e.g. "9.4.a.f.c.e.e.f.e.e.1.5.4.1.4.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.e.f.ip6.arpa" */
#define IPv6_REVERSE_LEN 72

/* index of the trailing char that must be "8", "9", "A", "a", "b", or "B" */
#define IPv6_REVERSE_LINK_LOCAL_TRAILING_CHAR 58

/* index of low-order nibble of embedded scope id */
#define IPv6_REVERSE_LINK_LOCAL_SCOPE_ID_LOW 48

const static uint8_t hexval[128] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,		/*  0 - 15 */
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,		/* 16 - 31 */
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,		/* 32 - 47 */
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0,  0,  0,  0,  0,		/* 48 - 63 */
	0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,		/* 64 - 79 */
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,		/* 80 - 95 */
	0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,		/* 96 - 111 */
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0		/* 112 - 127 */
};

static void
_mdns_debug_message(const char *str, ...)
{
	va_list v;
	char *out = NULL;
	if ((_mdns_debug & MDNS_DEBUG_OUT) == 0) return;

	va_start(v, str);
	vasprintf(&out, str, v);
	if (out == NULL) return;

	if (_mdns_debug & MDNS_DEBUG_STDOUT) fprintf(stdout, "%s", out);
	if (_mdns_debug & MDNS_DEBUG_STDERR) fprintf(stderr, "%s", out);
	if (_mdns_debug & MDNS_DEBUG_ASL) asl_log_message(ASL_LEVEL_NOTICE, "%s", out);
	free(out);

	va_end(v);
}

static char *
_mdns_reverse_ipv4(const char *addr)
{
	union
	{
		uint32_t a;
		unsigned char b[4];
	} ab;
	char *p;

	if (addr == NULL) return NULL;

	memcpy(&(ab.a), addr, 4);

	asprintf(&p, "%u.%u.%u.%u.in-addr.arpa.", ab.b[3], ab.b[2], ab.b[1], ab.b[0]);
	return p;
}

static char *
_mdns_reverse_ipv6(const char *addr)
{
	char x[65], *p;
	int i, j;
	u_int8_t d, hi, lo;

	if (addr == NULL) return NULL;

	x[64] = '\0';
	j = 63;
	for (i = 0; i < 16; i++)
	{
		d = addr[i];
		lo = d & 0x0f;
		hi = d >> 4;
		x[j--] = '.';
		x[j--] = hexchar[hi];
		x[j--] = '.';
		x[j--] = hexchar[lo];
	}

	asprintf(&p, "%sip6.arpa.", x);

	return p;
}

/* _mdns_canonicalize
 * Canonicalize the domain name by converting to lower case and removing the
 * trailing '.' if present.
 */
static char *
_mdns_canonicalize(const char *s)
{
	int i;
	char *t;
	if (s == NULL) return NULL;
	t = strdup(s);
	if (t == NULL) return NULL;
	if (t[0] == '\0') return t;
	for (i = 0; t[i] != '\0'; i++) {
		if (t[i] >= 'A' && t[i] <= 'Z') t[i] += 32;
	}
	if (t[i-1] == '.') t[i-1] = '\0';
	return t;
}

/* _mdns_hostent_append_alias
 * Appends an alias to the mdns_hostent_t structure.
 */
static int
_mdns_hostent_append_alias(mdns_hostent_t *h, const char *alias)
{
	int i;
	char *name;
	if (h == NULL || alias == NULL) return 0;
	name = _mdns_canonicalize(alias);
	if (name == NULL) return -1;

	// don't add the name if it matches an existing name
	if (h->host.h_name && string_equal(h->host.h_name, name)) {
		free(name);
		return 0;
	}
	for (i = 0; i < h->alias_count; ++i) {
		if (string_equal(h->host.h_aliases[i], name)) {
			free(name);
			return 0;
		}
	}

	// add the alias and NULL terminate the list
	h->host.h_aliases = (char **)reallocf(h->host.h_aliases, (h->alias_count+2) * sizeof(char *));
	if (h->host.h_aliases == NULL) {
		h->alias_count = 0;
		free(name);
		return -1;
	}
	h->host.h_aliases[h->alias_count] = name;
	++h->alias_count;
	h->host.h_aliases[h->alias_count] = NULL;
	return 0;
}

/* _mdns_hostent_append_addr
 * Appends an alias to the mdns_hostent_t structure.
 */
static int
_mdns_hostent_append_addr(mdns_hostent_t *h, const uint8_t *addr, uint32_t len)
{
	if (h == NULL || addr == NULL || len == 0) return 0;

	// copy the address buffer
	uint8_t *buf = malloc(len);
	if (buf == NULL) return -1;
	memcpy(buf, addr, len);

	// add the address and NULL terminate the list
	h->host.h_addr_list = (char **)reallocf(h->host.h_addr_list, (h->addr_count+2) * sizeof(char *));
	if (h->host.h_addr_list == NULL) {
		h->addr_count = 0;
		return -1;
	}
	h->host.h_addr_list[h->addr_count] = (char*)buf;
	h->addr_count++;
	h->host.h_addr_list[h->addr_count] = NULL;
	return 0;
}

static void
_mdns_hostent_clear(mdns_hostent_t *h)
{
	if (h == NULL) return;
	free(h->host.h_name);
	h->host.h_name = NULL;

	char **aliases = h->host.h_aliases;
	while (aliases && *aliases) {
		free(*aliases++);
	}
	free(h->host.h_aliases);
	h->host.h_aliases = NULL;
	h->alias_count = 0;

	char **addrs = h->host.h_addr_list;
	while (addrs && *addrs) {
		free(*addrs++);
	}
	free(h->host.h_addr_list);
	h->host.h_addr_list = NULL;
	h->addr_count = 0;

}

static void
_mdns_reply_clear(mdns_reply_t *r)
{
	if (r == NULL) return;
	r->ifnum = 0;
	_mdns_hostent_clear(r->h4);
	_mdns_hostent_clear(r->h6);
	mdns_srv_t *srv = r->srv;
	r->srv = NULL;
	while (srv) {
		mdns_srv_t *next = srv->next;
		free(srv->srv.target);
		free(srv);
		srv = next;
	}
}

static si_item_t *
mdns_hostbyname(si_mod_t *si, const char *name, int af, const char *interface, uint32_t *err)
{
	uint32_t type;
	mdns_hostent_t h;
	mdns_reply_t reply;
	si_item_t *out = NULL;
	uint64_t bb;
	int status;
	DNSServiceFlags flags = 0;

	if (err != NULL) *err = SI_STATUS_NO_ERROR;

	if (name == NULL) {
		if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
		return NULL;
	}

	memset(&h, 0, sizeof(h));
	memset(&reply, 0, sizeof(reply));

	switch (af) {
		case AF_INET:
			type = ns_t_a;
			h.host.h_length = 4;
			reply.h4 = &h;
			break;
		case AF_INET6:
			type = ns_t_aaaa;
			h.host.h_length = 16;
			reply.h6 = &h;
			break;
		default:
			if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
			return NULL;
	}
	h.host.h_addrtype = af;

	status = _mdns_search(name, ns_c_in, type, interface, flags, NULL, NULL, &reply);
	if (status != 0 || h.addr_count == 0) {
		_mdns_reply_clear(&reply);
		if (err != NULL) *err = SI_STATUS_H_ERRNO_HOST_NOT_FOUND;
		return NULL;
	}

	bb = reply.ttl + time(NULL);

	switch (af) {
		case AF_INET:
			out = (si_item_t *)LI_ils_create("L4488s*44a", (unsigned long)si, CATEGORY_HOST_IPV4, 1, bb, 0LL, h.host.h_name, h.host.h_aliases, h.host.h_addrtype, h.host.h_length, h.host.h_addr_list);
			break;
		case AF_INET6:
			out = (si_item_t *)LI_ils_create("L4488s*44c", (unsigned long)si, CATEGORY_HOST_IPV6, 1, bb, 0LL, h.host.h_name, h.host.h_aliases, h.host.h_addrtype, h.host.h_length, h.host.h_addr_list);
			break;
	}

	_mdns_reply_clear(&reply);

	if (out == NULL && err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;

	return out;
}

static si_item_t *
mdns_hostbyaddr(si_mod_t *si, const void *addr, int af, const char *interface, uint32_t *err)
{
	mdns_hostent_t h;
	mdns_reply_t reply;
	char *name;
	si_item_t *out;
	uint64_t bb;
	int cat;
	int status;
	DNSServiceFlags flags = 0;

	if (err != NULL) *err = SI_STATUS_NO_ERROR;

	if (addr == NULL || si == NULL) {
		if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
		return NULL;
	}

	memset(&h, 0, sizeof(h));
	memset(&reply, 0, sizeof(reply));

	switch (af) {
		case AF_INET:
			h.host.h_length = 4;
			reply.h4 = &h;
			name = _mdns_reverse_ipv4(addr);
			cat = CATEGORY_HOST_IPV4;
			break;
		case AF_INET6:
			h.host.h_length = 16;
			reply.h6 = &h;
			name = _mdns_reverse_ipv6(addr);
			cat = CATEGORY_HOST_IPV6;
			break;
		default:
			if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
			return NULL;
	}
	h.host.h_addrtype = af;

	status = _mdns_search(name, ns_c_in, ns_t_ptr, interface, flags, NULL, NULL, &reply);
	free(name);
	if (status != 0) {
		_mdns_reply_clear(&reply);
		if (err != NULL) *err = SI_STATUS_H_ERRNO_HOST_NOT_FOUND;
		return NULL;
	}

	status = _mdns_hostent_append_addr(&h, addr, h.host.h_length);
	if (status != 0) {
		_mdns_hostent_clear(&h);
		if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
		return NULL;
	}

	bb = reply.ttl + time(NULL);
	out = (si_item_t *)LI_ils_create("L4488s*44a", (unsigned long)si, cat, 1, bb, 0LL, h.host.h_name, h.host.h_aliases, h.host.h_addrtype, h.host.h_length, h.host.h_addr_list);

	_mdns_hostent_clear(&h);

	if (out == NULL && err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
	return out;
}

static si_list_t *
mdns_addrinfo(si_mod_t *si, const void *node, const void *serv, uint32_t family, uint32_t socktype, uint32_t proto, uint32_t flags, const char *interface, uint32_t *err)
{
	int wantv4 = 1;
	int wantv6 = 1;
	struct in_addr a4;
	struct in6_addr a6;
	mdns_hostent_t h4;
	mdns_hostent_t h6;
	mdns_reply_t reply;
	uint32_t type;
	uint16_t port;

	if (family == AF_INET6)
	{
		if ((flags & AI_V4MAPPED) == 0) wantv4 = 0;
	}
	else if (family == AF_INET)
	{
		wantv6 = 0;
	}
	else if (family != AF_UNSPEC)
	{
		return NULL;
	}

	if (err != NULL) *err = SI_STATUS_NO_ERROR;

	si_list_t *out = NULL;

	memset(&h4, 0, sizeof(h4));
	memset(&h6, 0, sizeof(h6));
	memset(&reply, 0, sizeof(reply));

	h4.host.h_addrtype = AF_INET;
	h4.host.h_length = 4;
	h6.host.h_addrtype = AF_INET6;
	h6.host.h_length = 16;

	if (wantv4 && wantv6) {
		type = 0;
		reply.h4 = &h4;
		reply.h6 = &h6;
	} else if (wantv4) {
		reply.h4 = &h4;
		type = ns_t_a;
	} else if (wantv6) {
		type = ns_t_aaaa;
		reply.h6 = &h6;
	} else {
		if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
		return NULL;
	}

	// service lookup
	if ((flags & AI_NUMERICSERV) != 0) {
		port = *(uint16_t *)serv;
	} else {
		if (_gai_serv_to_port(serv, proto, &port) != 0) {
			if (err) *err = SI_STATUS_EAI_NONAME;
			return NULL;
		}
	}

	// host lookup
	if ((flags & AI_NUMERICHOST) != 0) {
		char *cname = NULL;
		struct in_addr *p4 = NULL;
		struct in6_addr *p6 = NULL;
		if (family == AF_INET) {
			p4 = &a4;
			memcpy(p4, node, sizeof(a4));
		} else if (family == AF_INET6) {
			p6 = &a6;
			memcpy(p6, node, sizeof(a6));
		}
		out = si_addrinfo_list(si, flags, socktype, proto, p4, p6, port, 0, cname, cname);
	} else {
		DNSServiceFlags dns_flags = 0;
		if (flags & AI_ADDRCONFIG) {
			dns_flags |= kDNSServiceFlagsSuppressUnusable;
		}
		int res;
		res = _mdns_search(node, ns_c_in, type, interface, dns_flags, NULL, NULL, &reply);
		if (res == 0 && (h4.addr_count > 0 || h6.addr_count > 0)) {
			out = si_addrinfo_list_from_hostent(si, flags, socktype, proto,
												port, 0,
												(wantv4 ? &h4.host : NULL),
												(wantv6 ? &h6.host : NULL));
		} else if (err != NULL) {
			*err = SI_STATUS_EAI_NONAME;
		}
		_mdns_reply_clear(&reply);
	}
	return out;
}

static si_list_t *
mdns_srv_byname(si_mod_t* si, const char *qname, const char *interface, uint32_t *err)
{
	si_list_t *out = NULL;
	mdns_reply_t reply;
	mdns_srv_t *srv;
	int res;
	const uint64_t unused = 0;
	DNSServiceFlags flags = 0;

	if (err != NULL) *err = SI_STATUS_NO_ERROR;

	memset(&reply, 0, sizeof(reply));
	res = _mdns_search(qname, ns_c_in, ns_t_srv, interface, flags, NULL, NULL, &reply);
	if (res == 0) {
		srv = reply.srv;
		while (srv) {
			si_item_t *item;
			item = (si_item_t *)LI_ils_create("L4488222s", (unsigned long)si, CATEGORY_SRV, 1, unused, unused, srv->srv.priority, srv->srv.weight, srv->srv.port, srv->srv.target);
			out = si_list_add(out, item);
			si_item_release(item);
			srv = srv->next;
		}
	}
	_mdns_reply_clear(&reply);
	return out;
}

/*
 * We support dns_async_start / cancel / handle_reply using dns_item_call
 */
static si_item_t *
mdns_item_call(si_mod_t *si, int call, const char *name, const char *ignored, const char *interface, uint32_t class, uint32_t type, uint32_t *err)
{
	int res;
	uint8_t buf[DNS_MAX_RECEIVE_SIZE];
	uint32_t len = sizeof(buf);
	mdns_reply_t reply;
	mdns_hostent_t h4;
	mdns_hostent_t h6;
	si_item_t *out;
	DNSServiceFlags flags = 0;

	if (err != NULL) *err = SI_STATUS_NO_ERROR;

	if (name == NULL) {
		if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
		return NULL;
	}

	memset(&h4, 0, sizeof(h4));
	memset(&h6, 0, sizeof(h6));
	memset(&reply, 0, sizeof(reply));

	h4.host.h_addrtype = AF_INET;
	h4.host.h_length = 4;
	h6.host.h_addrtype = AF_INET6;
	h6.host.h_length = 16;
	reply.h4 = &h4;
	reply.h6 = &h6;

	res = _mdns_search(name, class, type, interface, flags, buf, &len, &reply);
	if (res != 0 || len <= 0 || len > DNS_MAX_RECEIVE_SIZE) {
		_mdns_reply_clear(&reply);
		if (err != NULL) *err = SI_STATUS_H_ERRNO_HOST_NOT_FOUND;
		return NULL;
	}

	struct sockaddr_in6 from;
	uint32_t fromlen = sizeof(from);
	memset(&from, 0, fromlen);
	from.sin6_len = fromlen;
	from.sin6_family = AF_INET6;
	from.sin6_addr.__u6_addr.__u6_addr8[15] = 1;
	if (reply.ifnum != 0) {
		from.sin6_addr.__u6_addr.__u6_addr16[0] = htons(0xfe80);
		from.sin6_scope_id = reply.ifnum;
	}

	out = (si_item_t *)LI_ils_create("L4488@@", (unsigned long)si, CATEGORY_DNSPACKET, 1, 0LL, 0LL, len, buf, fromlen, &from);
	if (out == NULL && err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;

	_mdns_reply_clear(&reply);

	return out;
}

static int
mdns_is_valid(si_mod_t *si, si_item_t *item)
{
	return 0;
}

static void
mdns_close(si_mod_t *si)
{
}

static void
_mdns_atfork_prepare(void)
{
	// acquire our lock so that we know all other threads have "drained"
	pthread_mutex_lock(&_mdns_mutex);
}

static void
_mdns_atfork_parent(void)
{
	// parent can simply resume
	pthread_mutex_unlock(&_mdns_mutex);
}

static void
_mdns_atfork_child(void)
{
	// child needs to force re-initialization
	_mdns_old_sdref = _mdns_sdref; // for later deallocation
	_mdns_sdref = NULL;
	pthread_mutex_unlock(&_mdns_mutex);
}

static void
_mdns_init(void)
{
	pthread_atfork(_mdns_atfork_prepare, _mdns_atfork_parent, _mdns_atfork_child);

	if (getenv("RES_DEBUG") != NULL) _mdns_debug |= MDNS_DEBUG_STDOUT;
	int fd = open(MDNS_DEBUG_FILE, O_RDONLY, 0);
	errno = 0;
	if (fd >= 0)
	{
		int i, n;
		char c[5];
		memset(c, 0, sizeof(c));
		n = read(fd, c, 4);

		for (i = 0; i < n; i++)
		{
			if ((c[i] == 'o') || (c[i] == 'O')) _mdns_debug |= MDNS_DEBUG_STDOUT;
			if ((c[i] == 'e') || (c[i] == 'E')) _mdns_debug |= MDNS_DEBUG_STDERR;
			if ((c[i] == 'a') || (c[i] == 'A')) _mdns_debug |= MDNS_DEBUG_ASL;
			if ((c[i] == 'm') || (c[i] == 'M')) _mdns_debug |= MDNS_DEBUG_MORE;
		}
	}
}

si_mod_t *
si_module_static_mdns(void)
{
	static const struct si_mod_vtable_s mdns_vtable =
	{
		.sim_close = &mdns_close,
		.sim_is_valid = &mdns_is_valid,
		.sim_host_byname = &mdns_hostbyname,
		.sim_host_byaddr = &mdns_hostbyaddr,
		.sim_item_call = &mdns_item_call,
		.sim_addrinfo = &mdns_addrinfo,
		.sim_srv_byname = &mdns_srv_byname,
	};

	static si_mod_t si =
	{
		.vers = 1,
		.refcount = 1,
		.flags = SI_MOD_FLAG_STATIC,

		.private = NULL,
		.vtable = &mdns_vtable,
	};

	static dispatch_once_t once;

	dispatch_once(&once, ^{
		si.name = strdup("mdns");
		_mdns_init();
	});

	return (si_mod_t*)&si;
}

/*
 * _mdns_parse_domain_name
 * Combine DNS labels to form a string.
 * DNSService API does not return compressed names.
 */
static char *
_mdns_parse_domain_name(const uint8_t *data, uint32_t datalen)
{
	int i = 0, j = 0;
	uint32_t len;
	uint32_t domainlen = 0;
	char *domain = NULL;

	if ((data == NULL) || (datalen == 0)) return NULL;

	// i: index into input data
	// j: index into output string
	while (datalen-- > 0) {
		len = data[i++];
		domainlen += (len + 1);
		domain = reallocf(domain, domainlen);
		if (domain == NULL) return NULL;
		if (len == 0) break;	// DNS root (NUL)
		if (j > 0) {
			domain[j++] = datalen ? '.' : '\0'; 
		}

		while ((len-- > 0) && (datalen--)) {
			if (data[i] == '.') {
				// special case: escape the '.' with a '\'
				domain = reallocf(domain, ++domainlen);
				if (domain == NULL) return NULL;
				domain[j++] = '\\';
			}
			domain[j++] = data[i++];
		}
	}
	domain[j] = '\0';

	return domain;
}

/*
 * _mdns_pack_domain_name
 * Format the string as packed DNS labels.
 * Only used for one string at a time, therefore no need for compression.
 */
static int
_mdns_pack_domain_name(const char* str, uint8_t *buf, size_t buflen) {
	int i = 0;
	uintptr_t len = 0;

	while (i < buflen) {
		// calculate length to next '.' or '\0'
		char *dot = strchr(str, '.');
		if (dot == NULL) dot = strchr(str, '\0');
		len = (dot - str);
		if (len > NS_MAXLABEL) return -1;
		// copy data for label
		buf[i++] = len;
		while (str < dot && i < buflen) {
			buf[i++] = *str++;
		}
		// skip past '.', break if '\0'
		if (*str++ == '\0') break;
	}

	if (i >= buflen) return -1;

	if (len > 0) {
		// no trailing dot - add a null label
		buf[i++] = 0;
		if (i >= buflen) return -1;
	}

	buf[i] = '\0';
	return i;
}

static int
_is_rev_link_local(const char *name)
{
	int len, i;

	if (name == NULL) return 0;

	len = strlen(name);
	if (len == 0) return 0;

	/* check for trailing '.' */
	if (name[len - 1] == '.') len--;

	if (len != IPv6_REVERSE_LEN) return 0;

	i = IPv6_REVERSE_LINK_LOCAL_TRAILING_CHAR;
	if ((name[i] != '8') && (name[i] != '9') && (name[i] != 'A') && (name[i] != 'a') && (name[i] != 'B') && (name[i] != 'b')) return 0;

	i = IPv6_REVERSE_LINK_LOCAL_TRAILING_CHAR + 1;
	if (strncasecmp(name + i, ".e.f.ip6.arpa", 13)) return 0;

	for (i = 0; i < IPv6_REVERSE_LINK_LOCAL_TRAILING_CHAR; i += 2)
	{
		if (name[i] < '0') return 0;
		if ((name[i] > '9') && (name[i] < 'A')) return 0;
		if ((name[i] > 'F') && (name[i] < 'a')) return 0;
		if (name[i] > 'f') return 0;
		if (name[i + 1] != '.') return 0;
	}

	return 1;
}

/* _mdns_ipv6_extract_scope_id
 * If the input string is a link local IPv6 address with an encoded scope id,
 * the scope id is extracted and a new string is constructed with the scope id removed.
 */
static char *
_mdns_ipv6_extract_scope_id(const char *name, uint32_t *out_ifnum)
{
	char *qname = NULL;
	uint16_t nibble;
	uint32_t iface;
	int i;

	if (out_ifnum != NULL) *out_ifnum = 0;

	/* examine the address, extract the scope id if present */
	if ((name != NULL) && (_is_rev_link_local(name)))
	{
		/* _is_rev_link_local rejects chars > 127 so it's safe to index into hexval */
		i = IPv6_REVERSE_LINK_LOCAL_SCOPE_ID_LOW;
		nibble = hexval[(uint32_t)name[i]];
		iface = nibble;

		i += 2;
		nibble = hexval[(uint32_t)name[i]];
		iface += (nibble << 4);

		i += 2;
		nibble = hexval[(uint32_t)name[i]];
		iface += (nibble << 8);

		i += 2;
		nibble = hexval[(uint32_t)name[i]];
		iface += (nibble << 12);

		if (iface != 0)
		{
			qname = strdup(name);
			if (qname == NULL) return NULL;

			i = IPv6_REVERSE_LINK_LOCAL_SCOPE_ID_LOW;
			qname[i] = '0';
			qname[i + 2] = '0';
			qname[i + 4] = '0';
			qname[i + 6] = '0';

			if (out_ifnum) *out_ifnum = iface;
		}
	}

	return qname;
}

static int
_mdns_make_query(const char* name, int class, int type, uint8_t *buf, uint32_t buflen)
{
	uint32_t len = 0;

	if (buf == NULL || buflen < (NS_HFIXEDSZ + NS_QFIXEDSZ)) return -1;
	memset(buf, 0, NS_HFIXEDSZ);
	HEADER *hp = (HEADER *)buf;

	len += NS_HFIXEDSZ;
	hp->id = arc4random();
	hp->qr = 1;
	hp->opcode = ns_o_query;
	hp->rd = 1;
	hp->rcode = ns_r_noerror;
	hp->qdcount = htons(1);

	int n = _mdns_pack_domain_name(name, &buf[len], buflen - len);
	if (n < 0) return -1;

	len += n;
	uint16_t word;
	word = htons(type);
	memcpy(&buf[len], &word, sizeof(word));
	len += sizeof(word);
	word = htons(class);
	memcpy(&buf[len], &word, sizeof(word));
	len += sizeof(word);
	return len;
}

typedef struct {
	mdns_reply_t *reply;
	mdns_hostent_t *host;
	uint8_t *answer; // DNS packet buffer
	size_t anslen; // DNS packet buffer current length
	size_t ansmaxlen; // DNS packet buffer maximum length
	int type; // type of query: A, AAAA, PTR, SRV...
	uint16_t last_type; // last type received
	uint32_t sd_gen;
	DNSServiceRef sd;
	DNSServiceFlags flags;
	DNSServiceErrorType error;
	int kq; // kqueue to notify when callback received
} mdns_query_context_t;

static void
_mdns_query_callback(DNSServiceRef, DNSServiceFlags, uint32_t, DNSServiceErrorType, const char *, uint16_t, uint16_t, uint16_t, const void *, uint32_t, void *);

/* _mdns_query_start
 * initializes the context and starts a DNS-SD query.
 */
static DNSServiceErrorType
_mdns_query_start(mdns_query_context_t *ctx, mdns_reply_t *reply, uint8_t *answer, uint32_t *anslen, const char* name, int class, int type, const char *interface, DNSServiceFlags flags, int kq)
{
	DNSServiceErrorType status;

	flags |= kDNSServiceFlagsShareConnection;
	flags |= kDNSServiceFlagsReturnIntermediates;

	/* <rdar://problem/7428439> mDNSResponder is now responsible for timeouts */
	flags |= kDNSServiceFlagsTimeout;

	memset(ctx, 0, sizeof(mdns_query_context_t));

	if (answer && anslen) {
		// build a dummy DNS header to return to the caller
		ctx->answer = answer;
		ctx->ansmaxlen = *anslen;
		ctx->anslen = _mdns_make_query(name, class, type, answer, ctx->ansmaxlen);
		if (ctx->anslen <= 0) return -1;
	}

	ctx->type = type;
	ctx->sd = _mdns_sdref;
	ctx->sd_gen = _mdns_generation;
	ctx->kq = kq;
	if (reply) {
		ctx->reply = reply;
		if (type == ns_t_a) ctx->host = reply->h4;
		else if (type == ns_t_aaaa) ctx->host = reply->h6;
		else if (type == ns_t_ptr && reply->h4) ctx->host = reply->h4;
		else if (type == ns_t_ptr && reply->h6) ctx->host = reply->h6;
		else if (type != ns_t_srv && type != ns_t_cname) return -1;
	}

	uint32_t iface = 0;
	char *qname = _mdns_ipv6_extract_scope_id(name, &iface);
	if (qname == NULL) qname = (char *)name;

	if (interface != NULL)
	{
		/* get interface number from name */
		int iface2 = if_nametoindex(interface);

		/* balk if interface name lookup failed */
		if (iface2 == 0) return -1;

		/* balk if scope id is set AND interface is given AND they don't match */
		if ((iface != 0) && (iface2 != 0) && (iface != iface2)) return -1;
		if (iface2 != 0) iface = iface2;
	}

	_mdns_debug_message(";; mdns query %s %d %d [ctx %p]\n", qname, type, class, ctx);

	status = DNSServiceQueryRecord(&ctx->sd, flags, iface, qname, type, class, _mdns_query_callback, ctx);
	if (qname != name) free(qname);
	return status;
}

/* _mdns_query_is_complete
 * Determines whether the specified query has sufficient information to be
 * considered complete.
 */
static int
_mdns_query_is_complete(mdns_query_context_t *ctx)
{
	int complete = 0;

	/* NULL context is an error, but we call it complete */
	if (ctx == NULL) return 1;

	/*
	 * The default is to ignore kDNSServiceFlagsMoreComing, since it has either
	 * never been supported or worked correctly.  MDNS_DEBUG_MORE makes us honor it.
	 */
	if (ctx->flags & kDNSServiceFlagsMoreComing) {
		if (_mdns_debug & MDNS_DEBUG_MORE) {
			_mdns_debug_message(";; mdns is_complete type %d ctx %p more coming - incomplete\n", ctx->type, ctx);
			return 0;
		}
	}

	if (ctx->last_type != ctx->type) {
		_mdns_debug_message(";; mdns is_complete ctx %p type mismatch (%d != %d) - incomplete\n", ctx, ctx->last_type, ctx->type);
		return 0;
	}

	switch (ctx->type) {
		case ns_t_a:
		case ns_t_aaaa:
			if (ctx->host != NULL && ctx->host->addr_count > 0) complete = 1;
			break;
		case ns_t_ptr:
			if (ctx->host != NULL && ctx->host->host.h_name != NULL) complete = 1;
			break;
		case ns_t_srv:
			if (ctx->reply != NULL && ctx->reply->srv != NULL) complete = 1;
			break;
		default:
			_mdns_debug_message(";; mdns is_complete unexpected type %d ctx %p\n", ctx->type, ctx);
	}

	_mdns_debug_message(";; mdns is_complete type %d ctx %p %s%scomplete\n", ctx->type, ctx, (ctx->flags & kDNSServiceFlagsMoreComing) ? "(more coming flag ignored)" : "", (complete == 0) ? " - in" : " - ");

	return complete;
}

/* _mdns_query_clear
 * Clear out the temporary fields of the context, and clear any result
 * structures that are incomplete.  Retrns 1 if the query was complete.
 */
static int
_mdns_query_clear(mdns_query_context_t *ctx)
{
	int complete = _mdns_query_is_complete(ctx);
	if (ctx == NULL) return complete;

	if (ctx->sd != NULL) {
		/* only dealloc this DNSServiceRef if the "main" _mdns_sdref has not been deallocated */
		if (ctx->sd != NULL && ctx->sd_gen == _mdns_generation) {
			DNSServiceRefDeallocate(ctx->sd);
		}
	}

	ctx->sd = NULL;
	ctx->sd_gen = 0;
	ctx->flags = 0;
	ctx->kq = -1;

	if (complete == 0) {
		_mdns_hostent_clear(ctx->host);
		ctx->anslen = -1;
	}

	return complete;
}

static void
_mdns_query_callback(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t ifIndex, DNSServiceErrorType errorCode, const char *fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void *rdata, uint32_t ttl, void *ctx)
{
	mdns_query_context_t *context;
	struct in6_addr a6;

	context = (mdns_query_context_t *)ctx;

	context->flags = flags;
	context->error = errorCode;
	context->last_type = rrtype;

	if (errorCode != kDNSServiceErr_NoError) {
		_mdns_debug_message(";; [%s %hu %hu]: error %d [ctx %p]\n", fullname, rrtype, rrclass, errorCode, context);
		goto wakeup_kevent;
	}

	// embed the scope ID into link-local IPv6 addresses
	if (rrtype == ns_t_aaaa && rdlen == sizeof(struct in6_addr) &&
	    IN6_IS_ADDR_LINKLOCAL((struct in6_addr *)rdata)) {
		memcpy(&a6, rdata, rdlen);
		a6.__u6_addr.__u6_addr16[1] = htons(ifIndex);
		rdata = &a6;
	}

	if (context->reply) {
		char *name;
		int malformed = 0;
		mdns_reply_t *reply = context->reply;

		if (reply->ifnum == 0) {
			reply->ifnum = ifIndex;
		}

		_mdns_hostent_append_alias(context->host, fullname);
		if (reply->ttl == 0 || ttl < reply->ttl) reply->ttl = ttl;

		switch (rrtype) {
			case ns_t_a:
			case ns_t_aaaa:
				if (((rrtype == ns_t_a && context->host->host.h_addrtype == AF_INET) ||
					 (rrtype == ns_t_aaaa && context->host->host.h_addrtype == AF_INET6)) &&
					rdlen >= context->host->host.h_length) {
					if (context->host->host.h_name == NULL) {
						int i;
						mdns_hostent_t *h = context->host;
						char *h_name = _mdns_canonicalize(fullname);
						context->host->host.h_name = h_name;

						// 6863416 remove h_name from h_aliases
						for (i = 0; i < h->alias_count; ++i) {
							if (h_name == NULL) break;
							if (string_equal(h->host.h_aliases[i], h_name)) {
								// includes trailing NULL pointer
								int sz = sizeof(char *) * (h->alias_count - i);
								free(h->host.h_aliases[i]);
								memmove(&h->host.h_aliases[i], &h->host.h_aliases[i+1], sz);
								h->alias_count -= 1;
								break;
							}
						}
					}
					_mdns_hostent_append_addr(context->host, rdata, context->host->host.h_length);
				} else {
					malformed = 1;
				}
				break;
			case ns_t_cname:
				name = _mdns_parse_domain_name(rdata, rdlen);
				if (!name) malformed = 1;
				_mdns_hostent_append_alias(context->host, name);
				_mdns_debug_message(";; [%s %hu %hu] cname %s [ctx %p]\n", fullname, rrtype, rrclass, name, context);
				free(name);
				break;
			case ns_t_ptr:
				name = _mdns_parse_domain_name(rdata, rdlen);
				if (!name) malformed = 1;
				if (context->host && context->host->host.h_name == NULL) {
					context->host->host.h_name = _mdns_canonicalize(name);
				}
				_mdns_hostent_append_alias(context->host, name);
				free(name);
				break;
			case ns_t_srv: {
				mdns_rr_srv_t *p = (mdns_rr_srv_t*)rdata;
				mdns_srv_t *srv = calloc(1, sizeof(mdns_srv_t));
				if (srv == NULL) break;
				if (rdlen < sizeof(mdns_rr_srv_t)) {
					malformed = 1;
					break;
				}
				srv->srv.priority = ntohs(p->priority);
				srv->srv.weight = ntohs(p->weight);
				srv->srv.port = ntohs(p->port);
				srv->srv.target = _mdns_parse_domain_name(&p->target[0], rdlen - 3*sizeof(uint16_t));
				if (srv->srv.target == NULL) {
					malformed = 1;
					break;
				}
				// append to the end of the list
				if (reply->srv == NULL) {
					reply->srv = srv;
				} else {
					mdns_srv_t *iter = reply->srv;
					while (iter->next) iter = iter->next;
					iter->next = srv;
				}
				break;
			}
			default:
				malformed = _mdns_debug;
				break;
		}
		if (malformed != 0) {
			_mdns_debug_message(";; [%s %hu %hu]: malformed reply [ctx %p]\n", fullname, rrtype, rrclass, context);
			goto wakeup_kevent;
		}
	}

	if (context->answer) {
		int n;
		uint8_t *cp;
		HEADER *ans;
		size_t buflen = context->ansmaxlen - context->anslen;
		if (buflen < NS_HFIXEDSZ) {
			_mdns_debug_message(";; [%s %hu %hu]: malformed reply (too small) [ctx %p]\n", fullname, rrtype, rrclass, context);
			goto wakeup_kevent;
		}

		cp = context->answer + context->anslen;

		n = _mdns_pack_domain_name(fullname, cp, buflen);
		if (n < 0) {
			_mdns_debug_message(";; [%s %hu %hu]: name mismatch [ctx %p]\n", fullname, rrtype, rrclass, context);
			goto wakeup_kevent;
		}

		// check that there is enough space in the buffer for the
		// resource name (n), the resource record data (rdlen) and
		// the resource record header (10).
		if (buflen < n + rdlen + 10) {
			_mdns_debug_message(";; [%s %hu %hu]: insufficient buffer space for reply [ctx %p]\n", fullname, rrtype, rrclass, context);
			goto wakeup_kevent;
		}

		cp += n;
		buflen -= n;

		uint16_t word;
		uint32_t longword;

		word = htons(rrtype);
		memcpy(cp, &word, sizeof(word));
		cp += sizeof(word);

		word = htons(rrclass);
		memcpy(cp, &word, sizeof(word));
		cp += sizeof(word);

		longword = htonl(ttl);
		memcpy(cp, &longword, sizeof(longword));
		cp += sizeof(longword);

		word = htons(rdlen);
		memcpy(cp, &word, sizeof(word));
		cp += sizeof(word);

		memcpy(cp, rdata, rdlen);
		cp += rdlen;

		ans = (HEADER *)context->answer;
		ans->ancount = htons(ntohs(ans->ancount) + 1);

		context->anslen = (size_t)(cp - context->answer);
	}

	_mdns_debug_message(";; [%s %hu %hu] reply [ctx %p]\n", fullname, rrtype, rrclass, context);

wakeup_kevent:
	// Ping the waiting thread in case this callback was invoked on another
	if (context->kq != -1) {
		struct kevent ev;
		EV_SET(&ev, 1, EVFILT_USER, 0, NOTE_TRIGGER, 0, 0);
		int res = kevent(context->kq, &ev, 1, NULL, 0, NULL);
		if (res != 0) _mdns_debug_message(";; kevent EV_TRIGGER: %s [ctx %p]\n", strerror(errno), context);
	}
}

static void
_mdns_now(struct timespec *now) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	now->tv_sec = tv.tv_sec;
	now->tv_nsec = tv.tv_usec * 1000;
}

static void
_mdns_add_time(struct timespec *sum, const struct timespec *a, const struct timespec *b)
{
	sum->tv_sec = a->tv_sec + b->tv_sec;
	sum->tv_nsec = a->tv_nsec + b->tv_nsec;
	if (sum->tv_nsec > 1000000000) {
		sum->tv_sec += (sum->tv_nsec / 1000000000);
		sum->tv_nsec %= 1000000000;
	}
}

// calculate a deadline from the current time based on the desired timeout
static void
_mdns_deadline(struct timespec *deadline, const struct timespec *delta)
{
	struct timespec now;
	_mdns_now(&now);
	_mdns_add_time(deadline, &now, delta);
}

static void
_mdns_sub_time(struct timespec *delta, const struct timespec *a, const struct timespec *b)
{
	delta->tv_sec = a->tv_sec - b->tv_sec;
	delta->tv_nsec = a->tv_nsec - b->tv_nsec;
	if (delta->tv_nsec < 0) {
		delta->tv_nsec += 1000000000;
		delta->tv_sec -= 1;
	}
}

// calculate a timeout remaining before the given deadline
static void
_mdns_timeout(struct timespec *timeout, const struct timespec *deadline)
{
	struct timespec now;
	_mdns_now(&now);
	_mdns_sub_time(timeout, deadline, &now);
}

int
_mdns_search(const char *name, int class, int type, const char *interface, DNSServiceFlags flags, uint8_t *answer, uint32_t *anslen, mdns_reply_t *reply)
{
	DNSServiceErrorType err = 0;
	int kq, n, wait = 1;
	struct kevent ev;
	struct timespec start, finish, delta, timeout;
	int res = 0;
	int i, complete, got_a_response = 0;
	int initialize = 1;
	uint32_t n_iface_4 = 0;

	// determine number of IPv4 interfaces (ignore loopback)
	si_inet_config(&n_iface_4, NULL);
	if (n_iface_4 > 0) n_iface_4--;

	// <rdar://problem/7732497> limit the number of initialization retries
	int initialize_retries = 3;

	// 2 for A and AAAA parallel queries
	int n_ctx = 0;
	mdns_query_context_t ctx[2];

	if (name == NULL) return -1;

#if TARGET_OS_EMBEDDED
	// log a warning for queries from the main thread 
	if (pthread_is_threaded_np() && pthread_main_np()) asl_log(NULL, NULL, ASL_LEVEL_WARNING, "Warning: Libinfo call to mDNSResponder on main thread");
#endif // TARGET_OS_EMBEDDED

	// Timeout Logic
	// The kevent(2) API timeout parameter is used to enforce the total
	// timeout of the DNS query.  Each iteraion recalculates the relative
	// timeout based on the desired end time (total timeout from origin).
	//
	// In order to workaround some DNS configurations that do not return
	// responses for AAAA queries, parallel queries modify the total
	// timeout upon receipt of the first response.  The new total timeout is
	// set to an effective value of 2N where N is the time taken to receive
	// the A response (the original total timeout is preserved if 2N would
	// have exceeded it).  However, since mDNSResponder caches values, a
	// minimum value of 50ms for N is enforced in order to give some time
	// for the receipt of a AAAA response.

	// determine the maximum time to wait for a result
	delta.tv_sec = RES_MAXRETRANS + 5;
	delta.tv_nsec = 0;
	_mdns_deadline(&finish, &delta);
	timeout = delta;
	_mdns_now(&start);

	for (i = 0; i < 2; ++i) {
		memset(&ctx[i], 0 , sizeof(mdns_query_context_t));
	}

	// set up the kqueue
	kq = kqueue();
	EV_SET(&ev, 1, EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, 0);
	n = kevent(kq, &ev, 1, NULL, 0, NULL);
	if (n != 0) wait = 0;

	while (wait == 1) {
		if (initialize) {
			initialize = 0;
			pthread_mutex_lock(&_mdns_mutex);
			// clear any stale contexts
			for (i = 0; i < n_ctx; ++i) {
				_mdns_query_clear(&ctx[i]);
			}
			n_ctx = 0;

			if (_mdns_sdref == NULL) {
				if (_mdns_old_sdref != NULL) {
					_mdns_generation++;
					DNSServiceRefDeallocate(_mdns_old_sdref);
					_mdns_old_sdref = NULL;
				}
				// (re)initialize the shared connection
				err = DNSServiceCreateConnection(&_mdns_sdref);

				// limit the number of retries
				if (initialize_retries-- <= 0 && err == 0) {
					err = kDNSServiceErr_Unknown;
				}
				if (err != 0) {
					wait = 0;
					pthread_mutex_unlock(&_mdns_mutex);
					break;
				}
			}

			// issue (or reissue) the queries
			// unspecified type: do parallel A and AAAA
			if (err == 0) {
				err = _mdns_query_start(&ctx[n_ctx++], reply,
										answer, anslen,
										name, class,
										(type == 0) ? ns_t_a : type, interface, flags, kq);
			}

			if (err == 0 && type == 0) {
				err = _mdns_query_start(&ctx[n_ctx++], reply,
										answer, anslen,
										name, class, ns_t_aaaa, interface, flags, kq);
			}

			if (err != 0) {
				_mdns_debug_message(";; initialization error %d\n", err);
			}

			// try to reinitialize
			if (err == kDNSServiceErr_Unknown ||
				err == kDNSServiceErr_ServiceNotRunning ||
				err == kDNSServiceErr_BadReference) {
				if (_mdns_sdref) {
					_mdns_generation++;
					DNSServiceRefDeallocate(_mdns_sdref);
					_mdns_sdref = NULL;
				}
				err = 0;
				initialize = 1;
				pthread_mutex_unlock(&_mdns_mutex);
				continue;
			} else if (err != 0) {
				pthread_mutex_unlock(&_mdns_mutex);
				break;
			}

			// (re)register the fd with kqueue
			int fd = DNSServiceRefSockFD(_mdns_sdref);
			EV_SET(&ev, fd, EVFILT_READ, EV_ADD, 0, 0, 0);
			n = kevent(kq, &ev, 1, NULL, 0, NULL);
			pthread_mutex_unlock(&_mdns_mutex);
			if (err != 0 || n != 0) break;
		}

		_mdns_debug_message(";; set kevent timeout %ld.%ld [ctx %p %p]\n", timeout.tv_sec, timeout.tv_nsec, (n_ctx > 0) ? &(ctx[0]) : NULL, (n_ctx > 1) ? &(ctx[1]) : NULL);

		n = kevent(kq, NULL, 0, &ev, 1, &timeout);
		if (n < 0 && errno != EINTR) {
			res = -1;
			break;
		}

		pthread_mutex_lock(&_mdns_mutex);
		// DNSServiceProcessResult() is a blocking API
		// confirm that there is still data on the socket
		const struct timespec notimeout = { 0, 0 };
		int m = kevent(kq, NULL, 0, &ev, 1, &notimeout);
		if (_mdns_sdref == NULL) {
			initialize = 1;
		} else if (m > 0 && ev.filter == EVFILT_READ) {
			err = DNSServiceProcessResult(_mdns_sdref);
			if (err == kDNSServiceErr_ServiceNotRunning ||
			    err == kDNSServiceErr_BadReference) {
				_mdns_debug_message(";; DNSServiceProcessResult status %d [ctx %p %p]\n", err, (n_ctx > 0) ? &(ctx[0]) : NULL, (n_ctx > 1) ? &(ctx[1]) : NULL);
				err = 0;
				// re-initialize the shared connection
				_mdns_generation++;
				DNSServiceRefDeallocate(_mdns_sdref);
				_mdns_sdref = NULL;
				initialize = 1;
			}
		}

		// Check if all queries are complete (including errors)
		complete = 1;
		for (i = 0; i < n_ctx; ++i) {
			if ((ctx[i].error != 0) || _mdns_query_is_complete(&ctx[i])) {
				if (ctx[i].type == ns_t_a) {
					got_a_response = GOT_DATA;
					if (ctx[i].error != 0) got_a_response = GOT_ERROR;
				}
				_mdns_debug_message(";; [%s %d %d] finished processing ctx %p\n", name, class, type, &(ctx[i]));

			} else {
				_mdns_debug_message(";; [%s %d %d] continuing ctx %p\n", name, class, type, &(ctx[i]));
				complete = 0;
			}
		}
		pthread_mutex_unlock(&_mdns_mutex);

		if (err != 0) {
			_mdns_debug_message(";; DNSServiceProcessResult error status %d [ctx %p %p]\n", err, (n_ctx > 0) ? &(ctx[0]) : NULL, (n_ctx > 1) ? &(ctx[1]) : NULL);
			break;
		} else if (complete == 1) {
			_mdns_debug_message(";; [%s %d %d] done [ctx %p %p]\n", name, class, type, (n_ctx > 0) ? &(ctx[0]) : NULL, (n_ctx > 1) ? &(ctx[1]) : NULL);
			break;
		} else if (got_a_response != 0) {
			// got A, adjust deadline for AAAA
			struct timespec now, tn, extra;

			// delta = now - start
			_mdns_now(&now);
			_mdns_sub_time(&delta, &now, &start);

			extra.tv_sec = SHORT_AAAA_EXTRA;
			extra.tv_nsec = 0;

			// if delta is small (<= 20 milliseconds), we probably got a result from mDNSResponder's cache
			if ((delta.tv_sec == 0) && (delta.tv_nsec <= 20000000)) {
				extra.tv_sec = MEDIUM_AAAA_EXTRA;
			}
			else if (n_iface_4 == 0) {
				extra.tv_sec = LONG_AAAA_EXTRA;
			} else if (got_a_response == GOT_ERROR) {
				extra.tv_sec = MEDIUM_AAAA_EXTRA;
			}

			// tn = 2 * delta
			_mdns_add_time(&tn, &delta, &delta);

			// delta = tn + extra
			_mdns_add_time(&delta, &tn, &extra);

			// check that delta doesn't exceed our total timeout
			_mdns_sub_time(&tn, &timeout, &delta);
			if (tn.tv_sec >= 0) {
				_mdns_debug_message(";; new timeout [%s %d %d] (waiting for AAAA) %ld.%ld [ctx %p %p]\n", name, class, type, delta.tv_sec, delta.tv_nsec, (n_ctx > 0) ? &(ctx[0]) : NULL, (n_ctx > 1) ? &(ctx[1]) : NULL);
				_mdns_deadline(&finish, &delta);
			}
		}

		// calculate remaining timeout
		_mdns_timeout(&timeout, &finish);

		// check for time remaining
		if (timeout.tv_sec < 0) {
			_mdns_debug_message(";; [%s %d %d] timeout [ctx %p %p]\n", name, class, type, (n_ctx > 0) ? &(ctx[0]) : NULL, (n_ctx > 1) ? &(ctx[1]) : NULL);
			break;
		}
	}

	complete = 0;
	pthread_mutex_lock(&_mdns_mutex);
	for (i = 0; i < n_ctx; ++i) {
		if (err == 0) err = ctx[i].error;
		// Only clears hostents if result is incomplete.
		complete = _mdns_query_clear(&ctx[i]) || complete;
	}
	pthread_mutex_unlock(&_mdns_mutex);
	// Everything should be done with the kq by now.
	close(kq);

	// Return error if everything is incomplete
	if (complete == 0) {
		res = -1;
	}

	if (anslen) *anslen = ctx[0].anslen;
	return res;
}
