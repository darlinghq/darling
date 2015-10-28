/*
 * Copyright (c) 1999-2007 Apple Inc. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include "dns.h"
#include "dns_util.h"
#include "dns_private.h"
#include "res_private.h"

#define DNS_RESOLVER_DIR "/etc/resolver"

#define DNS_PRIVATE_HANDLE_TYPE_SUPER 0
#define DNS_PRIVATE_HANDLE_TYPE_PLAIN 1
#define DNS_DEFAULT_RECEIVE_SIZE 8192
#define DNS_MAX_RECEIVE_SIZE 65536

#define SDNS_DEFAULT_STAT_LATENCY 10

#define DNS_FLAGS_QR_MASK  0x8000
#define DNS_FLAGS_QR_QUERY 0x0000

#define DNS_FLAGS_OPCODE_MASK    0x7800

#define DNS_FLAGS_RCODE_MASK 0x000f

#define DNS_FLAGS_AA 0x0400
#define DNS_FLAGS_TC 0x0200
#define DNS_FLAGS_RD 0x0100
#define DNS_FLAGS_RA 0x0080

#define DNS_SOCK_UDP 0
#define DNS_SOCK_TCP_UNCONNECTED 1
#define DNS_SOCK_TCP_CONNECTED 2

#define INET_NTOP_AF_INET_OFFSET 4
#define INET_NTOP_AF_INET6_OFFSET 8

#define MAXPACKET 1024

extern void res_client_close(res_state res);
extern int __res_nquery(res_state statp, const char *name, int class, int type, u_char *answer, int anslen);
extern int dns_res_send(res_state statp, const u_char *buf, int buflen, u_char *ans, int *anssiz, struct sockaddr *from, int *fromlen);
extern void _check_cache(sdns_handle_t *sdns);
extern int _sdns_search(sdns_handle_t *sdns, const char *name, uint32_t class, uint32_t type, uint32_t fqdn, uint32_t recurse, char *buf, uint32_t len, struct sockaddr *from, uint32_t *fromlen, int *min);
extern int _pdns_search(sdns_handle_t *sdns, pdns_handle_t *pdns, const char *name, uint32_t class, uint32_t type, char *buf, uint32_t len, struct sockaddr *from, uint32_t *fromlen);
static pthread_mutex_t _dnsPrintLock = PTHREAD_MUTEX_INITIALIZER;

static void
_dns_print_lock(void)
{
	pthread_mutex_lock(&_dnsPrintLock);
}

static void
_dns_print_unlock(void)
{
	pthread_mutex_unlock(&_dnsPrintLock);
}


static uint8_t
_dns_parse_uint8(char **p)
{
	uint8_t v;

	v = (uint8_t)**p;
	*p += 1;
	return v;
}

static uint16_t
_dns_parse_uint16(char **p)
{
	uint16_t *x, v;

	x = (uint16_t *)*p;
	v = ntohs(*x);
	*p += 2;
	return v;
}

static uint32_t
_dns_parse_uint32(char **p)
{
	uint32_t *x, v;

	x = (uint32_t *)*p;
	v = ntohl(*x);
	*p += 4;
	return v;
}

static uint8_t
_dns_cname_length(char *s)
{
	uint8_t l;

	if (s == NULL) return 1;
	l = strlen(s);
	while ((s[l - 1] == '.') && (l > 1)) l--;
	return l;
}

static void
_dns_insert_cname(char *s, char *p)
{
	int i;
	uint8_t len, dlen;

	if (s == NULL)
	{
		*p = 0;
		return;
	}

	if (!strcmp(s, "."))
	{
		p[0] = 1;
		p[1] = '.';
		p[2] = 0;
		return;
	}

	len = _dns_cname_length(s);

	p[0] = '.';
	memmove(p + 1, s, len);
	p[len + 1] = '.';

	dlen = 0;

	for (i = len + 1; i >= 0; i--)
	{
		if (p[i] == '.')
		{
			p[i] = dlen;
			dlen = 0;
		}
		else dlen++;
	}
}

static char *
_dns_parse_string(const char *p, char **x, int32_t *remaining)
{
	char *str;
	uint8_t len;

	if (*remaining < 1) return NULL;
	*remaining -= 1;

	len = (uint8_t)**x;
	*x += 1;

	if (*remaining < len) return NULL;
	*remaining -= len;

	str = malloc(len + 1);
	memmove(str, *x, len);
	str[len] = '\0';
	*x += len;

	return str;
}

static char *
_dns_parse_domain_name(const char *p, char **x, int32_t *remaining)
{
	uint8_t *v8;
	uint16_t *v16, skip;
	uint16_t i, j, dlen, len;
	int more, compressed;
	char *name, *start, *y, *z;

	if (*remaining < 1) return NULL;

	z = *x + *remaining;
	start = *x;
	compressed = 0;
	more = 1;
	name = malloc(1);
	name[0] = '\0';
	len = 1;
	j = 0;
	skip = 0;

	while (more == 1)
	{
		if ((*x + 1) > z)
		{
			free(name);
			return NULL;
		}

		v8 = (uint8_t *)*x;
		dlen = *v8;

		if ((dlen & 0xc0) == 0xc0)
		{
			if ((*x + 2) > z)
			{
				free(name);
				return NULL;
			}

			v16 = (uint16_t *)*x;

			y = (char *)p + (ntohs(*v16) & 0x3fff);
			if ((*x == y) || (y > z))
			{
				free(name);
				return NULL;
			}

			*x = y;
			if (compressed == 0) skip += 2;
			compressed = 1;
			continue;
		}

		if ((*x + 1) > z)
		{
			free(name);
			return NULL;
		}

		*x += 1;

		if (dlen > 0)
		{
			len += dlen;
			name = realloc(name, len);
		}

		if ((*x + dlen) > z)
		{
			free(name);
			return NULL;
		}

		for (i = 0; i < dlen; i++)
		{
			name[j++] = **x;
			*x += 1;
		}

		name[j] = '\0';
		if (compressed == 0) skip += (dlen + 1);

		if (dlen == 0) more = 0;
		else
		{
			if ((*x + 1) > z)
			{
				free(name);
				return NULL;
			}

			v8 = (uint8_t *)*x;
			if (*v8 != 0)
			{
				len += 1;
				name = realloc(name, len);
				name[j++] = '.';
				name[j] = '\0';
			}
		}
	}

	if ((start + skip) > z)
	{
		free(name);
		return NULL;
	}

	*x = start + skip;
	*remaining -= skip;

	return name;
}

dns_resource_record_t *
_dns_parse_resource_record_internal(const char *p, char **x, int32_t *remaining)
{
	uint32_t size, bx, mi;
	uint16_t rdlen;
	uint8_t byte, i;
	dns_resource_record_t *r;
	char *eor;

	if (*remaining < 1) return NULL;

	r = (dns_resource_record_t *)calloc(1, sizeof(dns_resource_record_t));

	r->name = _dns_parse_domain_name(p, x, remaining);
	if (r->name == NULL)
	{
		dns_free_resource_record(r);
		return NULL;
	}

	if (*remaining < 10)
	{
		dns_free_resource_record(r);
		return NULL;
	}

	r->dnstype = _dns_parse_uint16(x);
	r->dnsclass = _dns_parse_uint16(x);
	r->ttl = _dns_parse_uint32(x);
	rdlen = _dns_parse_uint16(x);

	*remaining -= 10;

	if (*remaining < rdlen)
	{
		dns_free_resource_record(r);
		return NULL;
	}

	eor = *x;
	r->data.A = NULL;

	switch (r->dnstype)
	{
		case ns_t_a:
			if (*remaining < 4)
			{
				dns_free_resource_record(r);
				return NULL;
			}

			*remaining -= 4;

			size = sizeof(dns_address_record_t);
			r->data.A = (dns_address_record_t *)calloc(1, size);
			r->data.A->addr.s_addr = htonl(_dns_parse_uint32(x));
			break;

		case ns_t_aaaa:
			if (*remaining < 16)
			{
				dns_free_resource_record(r);
				return NULL;
			}

			*remaining -= 16;

			size = sizeof(dns_in6_address_record_t);
			r->data.AAAA = (dns_in6_address_record_t *)calloc(1, size);
			r->data.AAAA->addr.__u6_addr.__u6_addr32[0] = htonl(_dns_parse_uint32(x));
			r->data.AAAA->addr.__u6_addr.__u6_addr32[1] = htonl(_dns_parse_uint32(x));
			r->data.AAAA->addr.__u6_addr.__u6_addr32[2] = htonl(_dns_parse_uint32(x));
			r->data.AAAA->addr.__u6_addr.__u6_addr32[3] = htonl(_dns_parse_uint32(x));
			break;

		case ns_t_ns:
		case ns_t_md:
		case ns_t_mf:
		case ns_t_cname:
		case ns_t_mb:
		case ns_t_mg:
		case ns_t_mr:
		case ns_t_ptr:
			size = sizeof(dns_domain_name_record_t);
			r->data.CNAME = (dns_domain_name_record_t *)calloc(1,size);
			r->data.CNAME->name = _dns_parse_domain_name(p, x, remaining);
			if (r->data.CNAME->name == NULL)
			{
				dns_free_resource_record(r);
				return NULL;
			}
			break;

		case ns_t_soa:
			size = sizeof(dns_SOA_record_t);
			r->data.SOA = (dns_SOA_record_t *)calloc(1, size);

			r->data.SOA->mname = _dns_parse_domain_name(p, x, remaining);
			if (r->data.SOA->mname == NULL)
			{
				dns_free_resource_record(r);
				return NULL;
			}

			r->data.SOA->rname = _dns_parse_domain_name(p, x, remaining);
			if (r->data.SOA->rname == NULL)
			{
				dns_free_resource_record(r);
				return NULL;
			}

			if (*remaining < 20) 
			{
				dns_free_resource_record(r);
				return NULL;
			}

			*remaining -= 20;

			r->data.SOA->serial = _dns_parse_uint32(x);
			r->data.SOA->refresh = _dns_parse_uint32(x);
			r->data.SOA->retry = _dns_parse_uint32(x);
			r->data.SOA->expire = _dns_parse_uint32(x);
			r->data.SOA->minimum = _dns_parse_uint32(x);
			break;

		case ns_t_wks:
			if (*remaining < 5) 
			{
				dns_free_resource_record(r);
				return NULL;
			}

			*remaining -= rdlen;

			size = sizeof(dns_WKS_record_t);
			r->data.WKS = (dns_WKS_record_t *)calloc(1, size);

			r->data.WKS->addr.s_addr = htonl(_dns_parse_uint32(x));
			r->data.WKS->protocol = _dns_parse_uint8(x);
			size = rdlen - 5;
			r->data.WKS->maplength = size * 8;
			r->data.WKS->map = NULL;
			if (size == 0) break;

			r->data.WKS->map = (uint8_t *)calloc(1, r->data.WKS->maplength);
			mi = 0;
			for (bx = 0; bx < size; bx++)
			{
				byte = _dns_parse_uint8(x);
				for (i = 128; i >= 1; i = i/2)
				{
					if (byte & i) r->data.WKS->map[mi] = 0xff;
					else r->data.WKS->map[mi] = 0;
					mi++;
				}
			}
			break;

		case ns_t_hinfo:
			size = sizeof(dns_HINFO_record_t);
			r->data.HINFO = (dns_HINFO_record_t *)calloc(1, size);

			r->data.HINFO->cpu = _dns_parse_string(p, x, remaining);
			if (r->data.HINFO->cpu == NULL)
			{
				dns_free_resource_record(r);
				return NULL;
			}

			r->data.HINFO->os = _dns_parse_string(p, x, remaining);
			if (r->data.HINFO->os == NULL)
			{
				dns_free_resource_record(r);
				return NULL;
			}

			break;

		case ns_t_minfo:
			size = sizeof(dns_MINFO_record_t);
			r->data.MINFO = (dns_MINFO_record_t *)calloc(1, size);

			r->data.MINFO->rmailbx = _dns_parse_domain_name(p, x, remaining);
			if (r->data.MINFO->rmailbx == NULL)
			{
				dns_free_resource_record(r);
				return NULL;
			}

			r->data.MINFO->emailbx = _dns_parse_domain_name(p, x, remaining);
			if (r->data.MINFO->emailbx == NULL)
			{
				dns_free_resource_record(r);
				return NULL;
			}

			break;

		case ns_t_mx:
			if (*remaining < 2) 
			{
				dns_free_resource_record(r);
				return NULL;
			}

			*remaining -= 2;

			size = sizeof(dns_MX_record_t);
			r->data.MX = (dns_MX_record_t *)calloc(1, size);

			r->data.MX->preference = _dns_parse_uint16(x);
			r->data.MX->name = _dns_parse_domain_name(p, x, remaining);
			if (r->data.MX->name == NULL)
			{
				dns_free_resource_record(r);
				return NULL;
			}

			break;

		case ns_t_txt:
			size = sizeof(dns_TXT_record_t);
			r->data.TXT = (dns_TXT_record_t *)malloc(size);
			r->data.TXT->string_count = 0;
			r->data.TXT->strings = NULL;

			while (*x < (eor + rdlen))
			{
				if (r->data.TXT->string_count == 0)
				{
					r->data.TXT->strings = (char **)calloc(1, sizeof(char *));
				}
				else
				{
					r->data.TXT->strings = (char **)realloc(r->data.TXT->strings, (r->data.TXT->string_count + 1) * sizeof(char *));
				}

				r->data.TXT->strings[r->data.TXT->string_count] = _dns_parse_string(p, x, remaining);
				if (r->data.TXT->strings[r->data.TXT->string_count] == NULL)
				{
					dns_free_resource_record(r);
					return NULL;
				}
				r->data.TXT->string_count++;
			}

			break;

		case ns_t_rp:
			size = sizeof(dns_RP_record_t);
			r->data.RP = (dns_RP_record_t *)calloc(1, size);

			r->data.RP->mailbox = _dns_parse_domain_name(p, x, remaining);
			if (r->data.RP->mailbox == NULL)
			{
				dns_free_resource_record(r);
				return NULL;
			}

			r->data.RP->txtdname = _dns_parse_domain_name(p, x, remaining);
			if (r->data.RP->txtdname == NULL)
			{
				dns_free_resource_record(r);
				return NULL;
			}

			break;

		case ns_t_afsdb:
			if (*remaining < 4) 
			{
				dns_free_resource_record(r);
				return NULL;
			}

			*remaining -= 4;
			size = sizeof(dns_AFSDB_record_t);
			r->data.AFSDB = (dns_AFSDB_record_t *)calloc(1, size);

			r->data.AFSDB->subtype = _dns_parse_uint32(x);
			r->data.AFSDB->hostname = _dns_parse_domain_name(p, x, remaining);
			if (r->data.AFSDB->hostname == NULL)
			{
				dns_free_resource_record(r);
				return NULL;
			}

			break;

		case ns_t_x25:
			size = sizeof(dns_X25_record_t);
			r->data.X25 = (dns_X25_record_t *)calloc(1, size);

			r->data.X25->psdn_address = _dns_parse_string(p, x, remaining);
			if (r->data.X25->psdn_address == NULL)
			{
				dns_free_resource_record(r);
				return NULL;
			}

			break;

		case ns_t_isdn:
			size = sizeof(dns_ISDN_record_t);
			r->data.ISDN = (dns_ISDN_record_t *)calloc(1, size);

			r->data.ISDN->isdn_address = _dns_parse_string(p, x, remaining);
			if (r->data.ISDN->isdn_address == NULL)
			{
				dns_free_resource_record(r);
				return NULL;
			}

			if (*x < (eor + rdlen))
			{
				r->data.ISDN->subaddress = _dns_parse_string(p, x, remaining);
				if (r->data.ISDN->subaddress == NULL)
				{
					dns_free_resource_record(r);
					return NULL;
				}
			}
			else
			{
				r->data.ISDN->subaddress = NULL;
			}

			break;

		case ns_t_rt:
			if (*remaining < 2) 
			{
				dns_free_resource_record(r);
				return NULL;
			}

			*remaining -= 2;

			size = sizeof(dns_RT_record_t);
			r->data.RT = (dns_RT_record_t *)calloc(1, size);

			r->data.RT->preference = _dns_parse_uint16(x);
			r->data.RT->intermediate = _dns_parse_domain_name(p, x, remaining);
			if (r->data.RT->intermediate == NULL)
			{
				dns_free_resource_record(r);
				return NULL;
			}

			break;

		case ns_t_loc:
			if (*remaining < 16) 
			{
				dns_free_resource_record(r);
				return NULL;
			}

			*remaining -= 16;

			size = sizeof(dns_LOC_record_t);
			r->data.LOC = (dns_LOC_record_t *)calloc(1, size);

			r->data.LOC->version = _dns_parse_uint8(x);
			r->data.LOC->size = _dns_parse_uint8(x);
			r->data.LOC->horizontal_precision = _dns_parse_uint8(x);
			r->data.LOC->vertical_precision = _dns_parse_uint8(x);
			r->data.LOC->latitude = _dns_parse_uint32(x);
			r->data.LOC->longitude = _dns_parse_uint32(x);
			r->data.LOC->altitude = _dns_parse_uint32(x);
			break;

		case ns_t_srv:
			if (*remaining < 6) 
			{
				dns_free_resource_record(r);
				return NULL;
			}

			*remaining -= 6;

			size = sizeof(dns_SRV_record_t);
			r->data.SRV = (dns_SRV_record_t *)calloc(1, size);

			r->data.SRV->priority = _dns_parse_uint16(x);
			r->data.SRV->weight = _dns_parse_uint16(x);
			r->data.SRV->port = _dns_parse_uint16(x);
			r->data.SRV->target = _dns_parse_domain_name(p, x, remaining);
			if (r->data.SRV->target == NULL)
			{
				dns_free_resource_record(r);
				return NULL;
			}

			break;

		case ns_t_null:
		default:
			*remaining -= rdlen;

			size = sizeof(dns_raw_resource_record_t);
			r->data.DNSNULL = (dns_raw_resource_record_t *)calloc(1, size);

			r->data.DNSNULL->length = rdlen;
			r->data.DNSNULL->data = calloc(1, rdlen);
			memmove(r->data.DNSNULL->data, *x, rdlen);
			*x += rdlen;
			break;
	}

	*x = eor + rdlen;
	return r;
}

dns_resource_record_t *
dns_parse_resource_record(const char *buf, uint32_t len)
{
	char *x;
	int32_t remaining;

	remaining = len;
	x = (char *)buf;
	return _dns_parse_resource_record_internal(buf, &x, &remaining);
}

dns_question_t *
_dns_parse_question_internal(const char *p, char **x, int32_t *remaining)
{
	dns_question_t *q;

	if (x == NULL) return NULL;
	if (*x == NULL) return NULL;
	if (*remaining < 1) return NULL;

	q = (dns_question_t *)calloc(1, sizeof(dns_question_t));

	q->name = _dns_parse_domain_name(p, x, remaining);
	if (q->name == NULL)
	{
		free(q);
		return NULL;
	}

	if (*remaining < 4)
	{
		free(q->name);
		free(q);
		return NULL;
	}

	*remaining = *remaining - 4;

	q->dnstype = _dns_parse_uint16(x);
	q->dnsclass = _dns_parse_uint16(x);

	return q;
}

dns_question_t *
dns_parse_question(const char *buf, uint32_t len)
{
	char *x;
	int32_t remaining;

	remaining = len;
	x = (char *)buf;
	return _dns_parse_question_internal(buf, &x, &remaining);
}


dns_reply_t *
dns_parse_packet(const char *p, uint32_t len)
{
	dns_reply_t *r;
	dns_header_t *h;
	char *x;
	uint32_t i, size;
    int32_t remaining;

	if (p == NULL) return NULL;
	if (len < NS_HFIXEDSZ) return NULL;

	x = (char *)p;

	r = (dns_reply_t *)calloc(1, sizeof(dns_reply_t));

	r->header = (dns_header_t *)calloc(1, sizeof(dns_header_t));
	h = r->header;

	h->xid = _dns_parse_uint16(&x);
	h->flags = _dns_parse_uint16(&x);
	h->qdcount = _dns_parse_uint16(&x);
	h->ancount = _dns_parse_uint16(&x);
	h->nscount = _dns_parse_uint16(&x);
	h->arcount = _dns_parse_uint16(&x);

	remaining = len - NS_HFIXEDSZ;

	size = sizeof(dns_question_t *);
	r->question = (dns_question_t **)calloc(h->qdcount, size);
	for (i = 0; i < h->qdcount; i++)
	{
		r->question[i] = _dns_parse_question_internal(p, &x, &remaining);
		if (r->question[i] ==NULL)
		{
			h->qdcount = 0;
			if (i > 0) h->qdcount = i - 1;
			h->ancount = 0;
			h->nscount = 0;
			h->arcount = 0;
			dns_free_reply(r);
			return NULL;
		}
	}

	size = sizeof(dns_resource_record_t *);

	r->answer = (dns_resource_record_t **)calloc(h->ancount, size);
	for (i = 0; i < h->ancount; i++)
	{
		r->answer[i] = _dns_parse_resource_record_internal(p, &x, &remaining);
		if (r->answer[i] == NULL)
		{
			h->ancount = 0;
			if (i > 0) h->ancount = i - 1;
			h->nscount = 0;
			h->arcount = 0;
			dns_free_reply(r);
			return NULL;
		}
	}

	r->authority = (dns_resource_record_t **)calloc(h->nscount, size);
	for (i = 0; i < h->nscount; i++)
	{
		r->authority[i] = _dns_parse_resource_record_internal(p, &x, &remaining);
		if (r->authority[i] == NULL)
		{
			h->nscount = 0;
			if (i > 0) h->nscount = i - 1;
			h->arcount = 0;
			dns_free_reply(r);
			return NULL;
		}
	}

	r->additional = (dns_resource_record_t **)calloc(h->arcount, size);
	for (i = 0; i < h->arcount; i++)
	{
		r->additional[i] = _dns_parse_resource_record_internal(p, &x, &remaining);
		if (r->additional[i] == NULL)
		{
			h->arcount = 0;
			if (i > 0) h->arcount = i - 1;
			dns_free_reply(r);
			return NULL;
		}
	}

	return r;
}

void
dns_free_resource_record(dns_resource_record_t *r)
{
	int i;

	free(r->name);

	switch (r->dnstype)
	{
		case ns_t_a:
			if (r->data.A != NULL) free(r->data.A);
			break;

		case ns_t_aaaa:
			if (r->data.AAAA != NULL) free(r->data.AAAA);
			break;

		case ns_t_ns:
		case ns_t_md:
		case ns_t_mf:
		case ns_t_cname:
		case ns_t_mb:
		case ns_t_mg:
		case ns_t_mr:
		case ns_t_ptr:
			if (r->data.CNAME != NULL)
			{
				if (r->data.CNAME->name != NULL) free(r->data.CNAME->name);
				free(r->data.CNAME);
			}
			break;

		case ns_t_soa:
			if (r->data.SOA != NULL)
			{
				if (r->data.SOA->mname != NULL) free(r->data.SOA->mname);
				if (r->data.SOA->rname != NULL) free(r->data.SOA->rname);
				free(r->data.SOA);
			}
			break;

		case ns_t_wks:
			if (r->data.WKS != NULL)
			{
				if (r->data.WKS->map != NULL) free(r->data.WKS->map);
				free(r->data.WKS);
			}
			break;

		case ns_t_hinfo:
			if (r->data.HINFO != NULL)
			{
				if (r->data.HINFO->cpu != NULL) free(r->data.HINFO->cpu);
				if (r->data.HINFO->os != NULL) free(r->data.HINFO->os);
				free(r->data.HINFO);
			}
			break;

		case ns_t_minfo:
			if (r->data.MINFO != NULL)
			{
				if (r->data.MINFO->rmailbx != NULL) free(r->data.MINFO->rmailbx);
				if (r->data.MINFO->emailbx != NULL) free(r->data.MINFO->emailbx);
				free(r->data.MINFO);
			}
			break;

		case ns_t_mx:
			if (r->data.MX != NULL)
			{
				if (r->data.MX->name != NULL) free(r->data.MX->name);
				free(r->data.MX);
			}
			break;


		case ns_t_txt:
			if (r->data.TXT != NULL)
			{
				for (i = 0; i < r->data.TXT->string_count; i++) free(r->data.TXT->strings[i]);
				if (r->data.TXT->strings != NULL) free(r->data.TXT->strings);
				free(r->data.TXT);
			}
			break;

		case ns_t_rp:
			if (r->data.RP != NULL)
			{
				if (r->data.RP->mailbox != NULL) free(r->data.RP->mailbox);
				if (r->data.RP->txtdname != NULL) free(r->data.RP->txtdname);
				free(r->data.RP);
			}
			break;

		case ns_t_afsdb:
			if (r->data.AFSDB != NULL)
			{
				if (r->data.AFSDB->hostname != NULL) free(r->data.AFSDB->hostname);
				free(r->data.AFSDB);
			}
			break;

		case ns_t_x25:
			if (r->data.X25 != NULL)
			{
				if (r->data.X25->psdn_address != NULL) free(r->data.X25->psdn_address);
				free(r->data.X25);
			}
			break;

		case ns_t_isdn:
			if (r->data.ISDN != NULL)
			{
				if (r->data.ISDN->isdn_address != NULL) free(r->data.ISDN->isdn_address);
				if (r->data.ISDN->subaddress != NULL) free(r->data.ISDN->subaddress);
				free(r->data.ISDN);
			}
			break;

		case ns_t_rt:
			if (r->data.RT != NULL)
			{
				if (r->data.RT->intermediate != NULL) free(r->data.RT->intermediate);
				free(r->data.RT);
			}
			break;

		case ns_t_loc:
			if (r->data.LOC != NULL) free(r->data.LOC);
			break;

		case ns_t_srv:
			if (r->data.SRV != NULL)
			{
				if (r->data.SRV->target != NULL) free(r->data.SRV->target);
				free(r->data.SRV);
			}
			break;

		case ns_t_invalid:
			break;

		case ns_t_null:
		default:
			if (r->data.DNSNULL != NULL)
			{
				if (r->data.DNSNULL->data != NULL) free(r->data.DNSNULL->data);
				free(r->data.DNSNULL);
			}
			break;
	}

	free(r);
}

void
dns_free_reply(dns_reply_t *r)
{
	uint32_t i;

	if (r == NULL) return;
	if (r->header != NULL)
	{
		for (i = 0; i < r->header->qdcount; i++)
		{
			free(r->question[i]->name);
			free(r->question[i]);
		}

		for (i = 0; i < r->header->ancount; i++) dns_free_resource_record(r->answer[i]);
		for (i = 0; i < r->header->nscount; i++) dns_free_resource_record(r->authority[i]);
		for (i = 0; i < r->header->arcount; i++) dns_free_resource_record(r->additional[i]);

		free(r->header);
	}

	if (r->question != NULL) free(r->question);
	if (r->answer != NULL) free(r->answer);
	if (r->authority != NULL) free(r->authority);
	if (r->additional != NULL) free(r->additional);

	if (r->server != NULL) free(r->server);

	free(r);
}

static void
_dns_append_question(dns_question_t *q, char **s, uint16_t *l)
{
	uint16_t len, *p;
	char *x;

	if (q == NULL) return;

	len = *l + _dns_cname_length(q->name) + 2 + 4;
	*s = realloc(*s, len);

	_dns_insert_cname(q->name, (char *)*s + *l);
	*l = len;

	x = *s + (len - 4);

	p = (uint16_t *)x;
	*p = htons(q->dnstype);
	x += 2;

	p = (uint16_t *)x;
	*p = htons(q->dnsclass);

}

static void
_dns_append_resource_record(dns_resource_record_t *r, char **s, uint16_t *l)
{
	uint16_t clen, len, *p, extra, rdlen;
	uint32_t *p2;
	char *x;

	if (r == NULL) return;

	extra = 10;
	switch (r->dnstype)
	{
		case ns_t_a:
			extra += 4;
			break;
		case ns_t_ptr:
			extra += 2;
			clen = _dns_cname_length(r->data.PTR->name);
			extra += clen;
			break;
		default: break;
	}

	len = *l + _dns_cname_length(r->name) + 2 + extra;
	*s = realloc(*s, len);

	_dns_insert_cname(r->name, (char *)*s + *l);
	*l = len;

	x = *s + (len - extra);

	p = (uint16_t *)x;
	*p = htons(r->dnstype);
	x += 2;

	p = (uint16_t *)x;
	*p = htons(r->dnsclass);
	x += 2;

	p2 = (uint32_t *)x;
	*p2 = htonl(r->ttl);
	x += 4;

	switch (r->dnstype)
	{
		case ns_t_a:
			rdlen = 4;
			p = (uint16_t *)x;
			*p = htons(rdlen);
			x += 2;

			p2 = (uint32_t *)x;
			*p2 = htons(r->data.A->addr.s_addr);
			x += 4;
			return;

		case ns_t_ptr:
			clen = _dns_cname_length(r->data.PTR->name) + 2;
			p = (uint16_t *)x;
			*p = htons(clen);
			x += 2;
			_dns_insert_cname(r->data.PTR->name, x);
			x += clen;
			return;

		default: return;
	}
}

char *
dns_build_reply(dns_reply_t *dnsr, uint16_t *rl)
{
	uint16_t i, len;
	dns_header_t *h;
	char *s, *x;

	if (dnsr == NULL) return NULL;

	len = NS_HFIXEDSZ;

	s = malloc(len);
	x = s + len;

	memset(s, 0, len);
	*rl = len;

	h = (dns_header_t *)s;

	h->xid = htons(dnsr->header->xid);
	h->flags = htons(dnsr->header->flags);
	h->qdcount = htons(dnsr->header->qdcount);
	h->ancount = htons(dnsr->header->ancount);
	h->nscount = htons(dnsr->header->nscount);
	h->arcount = htons(dnsr->header->arcount);

	for (i = 0; i < dnsr->header->qdcount; i++)
	{
		_dns_append_question(dnsr->question[i], &s, rl);
	}

	for (i = 0; i < dnsr->header->ancount; i++)
	{
		_dns_append_resource_record(dnsr->answer[i], &s, rl);
	}

	for (i = 0; i < dnsr->header->nscount; i++)
	{
		_dns_append_resource_record(dnsr->authority[i], &s, rl);
	}

	for (i = 0; i < dnsr->header->arcount; i++)
	{
		_dns_append_resource_record(dnsr->additional[i], &s, rl);
	}

	return s;
}

void
dns_free_question(dns_question_t *q)
{
	if (q == NULL) return;
	if (q->name != NULL) free(q->name);
	free(q);
}

void
dns_set_buffer_size(dns_handle_t d, uint32_t len)
{
	dns_private_handle_t *dns;
	if (d == NULL) return;

	dns = (dns_private_handle_t *)d;
	if (dns->recvsize == len) return;

	if (dns->recvbuf != NULL)
	{
		free(dns->recvbuf);
		dns->recvbuf = NULL;
	}

	dns->recvsize = len;
	if (dns->recvsize > DNS_MAX_RECEIVE_SIZE) dns->recvsize = DNS_MAX_RECEIVE_SIZE;

	if (dns->recvsize > 0) dns->recvbuf = malloc(dns->recvsize);
}

uint32_t
dns_get_buffer_size(dns_handle_t d)
{
	dns_private_handle_t *dns;
	if (d == NULL) return 0;

	dns = (dns_private_handle_t *)d;
	return dns->recvsize;
}

dns_reply_t *
dns_lookup_soa_min(dns_handle_t d, const char *name, uint32_t class, uint32_t type, int *min)
{
	dns_private_handle_t *dns;
	dns_reply_t *r;
	int len;
	struct sockaddr_storage *from;
	uint32_t fromlen;

	if (d == NULL) return NULL;
	if (name == NULL) return NULL;

	dns = (dns_private_handle_t *)d;
	if (min != NULL) *min = -1;

	if (dns->recvbuf == NULL)
	{
		if (dns->recvsize == 0) dns->recvsize = DNS_DEFAULT_RECEIVE_SIZE;

		dns->recvbuf = malloc(dns->recvsize);
		if (dns->recvbuf == NULL) return NULL;
	}

	fromlen = sizeof(struct sockaddr_storage);
	from = (struct sockaddr_storage *)calloc(1, sizeof(struct sockaddr_storage));
	len = -1;

	if (dns->handle_type == DNS_PRIVATE_HANDLE_TYPE_SUPER)
	{
		_check_cache(dns->sdns);
		len = _sdns_search(dns->sdns, name, class, type, 0, 1,  dns->recvbuf, dns->recvsize, (struct sockaddr *)from, &fromlen, min);
	}
	else
	{
		/* NB.  Minumium SOA TTL values are NOT provided when the caller passes a DNS_PRIVATE_HANDLE_TYPE_PLAIN handle */
		len = _pdns_search(dns->sdns, dns->pdns, name, class, type,  dns->recvbuf, dns->recvsize, (struct sockaddr *)from, &fromlen);
	}

	if (len <= 0)
	{
		free(from);
		return NULL;
	}

	r = dns_parse_packet(dns->recvbuf, len);

	if (r == NULL) free(from);
	else r->server = (struct sockaddr *)from;

	return r;
}

dns_reply_t *
dns_lookup(dns_handle_t d, const char *name, uint32_t class, uint32_t type)
{
	int unused = 0;

	return dns_lookup_soa_min(d, name, class, type, &unused);
}

/*
 * DNS printing utilities
 */

static char *
coord_ntoa(int32_t coord, uint32_t islat)
{
	int32_t deg, min, sec, secfrac;
	static char buf[64];
	char dir;

	coord = coord - 0x80000000;
	dir = 'N';

	if ((islat == 1) && (coord < 0))
	{
		dir = 'S';
		coord = -coord;
	}

	if (islat == 0)
	{
		dir = 'E';
		if (coord < 0)
		{
			dir = 'W';
			coord = -coord;
		}
	}

	secfrac = coord % 1000;
	coord = coord / 1000;
	sec = coord % 60;
	coord = coord / 60;
	min = coord % 60;
	coord = coord / 60;
	deg = coord;

	sprintf(buf, "%d %.2d %.2d.%.3d %c", deg, min, sec, secfrac, dir);
	return buf;
}

static char *
alt_ntoa(int32_t alt)
{
	int32_t ref, m, frac, sign;
	static char buf[128];

	ref = 100000 * 100;
	sign = 1;

	if (alt < ref)
	{
		alt = ref - alt;
		sign = -1;
	}
	else
	{
		alt = alt - ref;
	}

	frac = alt % 100;
	m = (alt / 100) * sign;

	sprintf(buf, "%d.%.2d", m, frac);
	return buf;
}

static unsigned int
poweroften[10] =
{	1,
	10,
	100,
	1000,
	10000,
	100000,
	1000000,
	10000000,
	100000000,
	1000000000
};

static char *
precsize_ntoa(uint8_t prec)
{
	static char buf[19];
	unsigned long val;
	int mantissa, exponent;

	mantissa = (int)((prec >> 4) & 0x0f) % 10;
	exponent = (int)((prec >> 0) & 0x0f) % 10;

	val = mantissa * poweroften[exponent];

	sprintf(buf, "%ld.%.2ld", val/100, val%100);
	return buf;
}

const char *
dns_type_string(uint16_t t)
{
	switch (t)
	{
		case ns_t_a:     return "A    ";
		case ns_t_ns:    return "NS   ";
		case ns_t_md:    return "MD   ";
		case ns_t_mf:    return "MF   ";
		case ns_t_cname: return "CNAME";
		case ns_t_soa:   return "SOA  ";
		case ns_t_mb:    return "MB  ";
		case ns_t_mg:    return "MG   ";
		case ns_t_mr:    return "MR   ";
		case ns_t_null:  return "NULL ";
		case ns_t_wks:   return "WKS  ";
		case ns_t_ptr:   return "PTR  ";
		case ns_t_hinfo: return "HINFO";
		case ns_t_minfo: return "MINFO";
		case ns_t_mx:    return "MX   ";
		case ns_t_txt:   return "TXT  ";
		case ns_t_rp:    return "PR   ";
		case ns_t_afsdb: return "AFSDB";
		case ns_t_x25:   return "X25  ";
		case ns_t_isdn:  return "ISDN ";
		case ns_t_rt:    return "RT   ";
		case ns_t_nsap:  return "NSAP ";
		case ns_t_nsap_ptr: return "NSPTR";
		case ns_t_sig:   return "SIG  ";
		case ns_t_key:   return "KEY  ";
		case ns_t_px:    return "PX   ";
		case ns_t_gpos:  return "GPOS ";
		case ns_t_aaaa:  return "AAAA ";
		case ns_t_loc:   return "LOC  ";
		case ns_t_nxt:   return "NXT  ";
		case ns_t_eid:   return "EID  ";
		case ns_t_nimloc: return "NIMLC";
		case ns_t_srv:   return "SRV  ";
		case ns_t_atma:  return "ATMA ";
		case ns_t_naptr: return "NAPTR";
		case ns_t_kx:    return "KX   ";
		case ns_t_cert:  return "CERT ";
		case ns_t_a6:    return "A6   ";
		case ns_t_dname: return "DNAME";
		case ns_t_sink:  return "SINK ";
		case ns_t_opt:   return "OPT  ";
		case ns_t_tkey:  return "TKEY ";
		case ns_t_tsig:  return "TSIG ";
		case ns_t_ixfr:  return "IXFR ";
		case ns_t_axfr:  return "AXFR ";
		case ns_t_mailb: return "MAILB";
		case ns_t_maila: return "MAILA";
		case ns_t_any:   return "ANY  ";
		case ns_t_zxfr:  return "ZXFR ";
		default:         return "?????";
	}

	return "?????";
}

int32_t
dns_type_number(const char *t, uint16_t *n)
{
	if (t == NULL) return -1;

	if (!strcasecmp(t, "A"))       { *n = ns_t_a;        return 0; }
	if (!strcasecmp(t, "NS"))      { *n = ns_t_ns;       return 0; }
	if (!strcasecmp(t, "MD"))      { *n = ns_t_md;       return 0; }
	if (!strcasecmp(t, "MF"))      { *n = ns_t_mf;       return 0; }
	if (!strcasecmp(t, "CNAME"))   { *n = ns_t_cname;    return 0; }
	if (!strcasecmp(t, "SOA"))     { *n = ns_t_soa;      return 0; }
	if (!strcasecmp(t, "MB"))      { *n = ns_t_mb;       return 0; }
	if (!strcasecmp(t, "MG"))      { *n = ns_t_mg;       return 0; }
	if (!strcasecmp(t, "MR"))      { *n = ns_t_mr;       return 0; }
	if (!strcasecmp(t, "NULL"))    { *n = ns_t_null;     return 0; }
	if (!strcasecmp(t, "WKS"))     { *n = ns_t_wks;      return 0; }
	if (!strcasecmp(t, "PTR"))     { *n = ns_t_ptr;      return 0; }
	if (!strcasecmp(t, "HINFO"))   { *n = ns_t_hinfo;    return 0; }
	if (!strcasecmp(t, "MINFO"))   { *n = ns_t_minfo;    return 0; }
	if (!strcasecmp(t, "MX"))      { *n = ns_t_mx;       return 0; }
	if (!strcasecmp(t, "TXT"))     { *n = ns_t_txt;      return 0; }
	if (!strcasecmp(t, "RP"))      { *n = ns_t_rp;       return 0; }
	if (!strcasecmp(t, "AFSDB"))   { *n = ns_t_afsdb;    return 0; }
	if (!strcasecmp(t, "X25"))     { *n = ns_t_x25;      return 0; }
	if (!strcasecmp(t, "ISDN"))    { *n = ns_t_isdn;     return 0; }
	if (!strcasecmp(t, "RT"))      { *n = ns_t_rt;       return 0; }
	if (!strcasecmp(t, "NSAP"))    { *n = ns_t_nsap;     return 0; }
	if (!strcasecmp(t, "NSPTR"))   { *n = ns_t_nsap_ptr; return 0; }
	if (!strcasecmp(t, "NSAP_PTR")){ *n = ns_t_nsap_ptr; return 0; }
	if (!strcasecmp(t, "SIG"))     { *n = ns_t_sig;      return 0; }
	if (!strcasecmp(t, "KEY"))     { *n = ns_t_key;      return 0; }
	if (!strcasecmp(t, "PX"))      { *n = ns_t_px;       return 0; }
	if (!strcasecmp(t, "GPOS"))    { *n = ns_t_gpos;     return 0; }
	if (!strcasecmp(t, "AAAA"))    { *n = ns_t_aaaa;     return 0; }
	if (!strcasecmp(t, "LOC"))     { *n = ns_t_loc;      return 0; }
	if (!strcasecmp(t, "NXT"))     { *n = ns_t_nxt;      return 0; }
	if (!strcasecmp(t, "EID"))     { *n = ns_t_eid;      return 0; }
	if (!strcasecmp(t, "NIMLOC"))  { *n = ns_t_nimloc;   return 0; }
	if (!strcasecmp(t, "SRV"))     { *n = ns_t_srv;      return 0; }
	if (!strcasecmp(t, "ATMA"))    { *n = ns_t_atma;     return 0; }
	if (!strcasecmp(t, "NAPTR"))   { *n = ns_t_naptr;    return 0; }
	if (!strcasecmp(t, "KX"))      { *n = ns_t_kx;       return 0; }
	if (!strcasecmp(t, "CERT"))    { *n = ns_t_cert;     return 0; }
	if (!strcasecmp(t, "A6"))      { *n = ns_t_a6;       return 0; }
	if (!strcasecmp(t, "DNAME"))   { *n = ns_t_dname;    return 0; }
	if (!strcasecmp(t, "SINK"))    { *n = ns_t_sink;     return 0; }
	if (!strcasecmp(t, "OPT"))     { *n = ns_t_opt;      return 0; }
	if (!strcasecmp(t, "TKEY"))    { *n = ns_t_tkey;     return 0; }
	if (!strcasecmp(t, "TSIG"))    { *n = ns_t_tsig;     return 0; }
	if (!strcasecmp(t, "IXFR"))    { *n = ns_t_ixfr;     return 0; }
	if (!strcasecmp(t, "AXFR"))    { *n = ns_t_axfr;     return 0; }
	if (!strcasecmp(t, "MAILB"))   { *n = ns_t_mailb;    return 0; }
	if (!strcasecmp(t, "MAILA"))   { *n = ns_t_maila;    return 0; }
	if (!strcasecmp(t, "ANY"))     { *n = ns_t_any;      return 0; }
	if (!strcasecmp(t, "ZXFR"))    { *n = ns_t_zxfr;     return 0; }

	return -1;
}

const char *
dns_class_string(uint16_t c)
{
	switch (c)
	{
		case ns_c_in: return "IN";
		case ns_c_2: return "CS";
		case ns_c_chaos: return "CH";
		case ns_c_hs: return "HS";
		case ns_c_none: return "NONE";
		case ns_c_any: return "ANY";
		default: return "??";
	}

	return "??";
}

int32_t
dns_class_number(const char *c, uint16_t *n)
{
	if (c == NULL) return -1;

	if (!strcasecmp(c, "IN"))   { *n = ns_c_in;    return 0; }
	if (!strcasecmp(c, "CS"))   { *n = ns_c_2;     return 0; }
	if (!strcasecmp(c, "CH"))   { *n = ns_c_chaos; return 0; }
	if (!strcasecmp(c, "HS"))   { *n = ns_c_hs;    return 0; }
	if (!strcasecmp(c, "NONE")) { *n = ns_c_none;  return 0; }
	if (!strcasecmp(c, "ANY"))  { *n = ns_c_any;   return 0; }

	return -1;
}

static void
_dns_print_question_lock(const dns_question_t *q, FILE *f, int lockit)
{
	if (lockit != 0) _dns_print_lock();
	fprintf(f, "%s %s %s\n", q->name, dns_class_string(q->dnsclass), dns_type_string(q->dnstype));
	if (lockit != 0) _dns_print_unlock();
}

void
dns_print_question(const dns_question_t *q, FILE *f)
{
	_dns_print_question_lock(q, f, 1);
}

static void
_dns_print_resource_record_lock(const dns_resource_record_t *r, FILE *f, int lockit)
{
	struct protoent *p;
	struct servent *s;
	uint32_t i, len;
	uint8_t x;
	struct sockaddr_in6 s6;
	char pbuf[64];

	if (lockit != 0) _dns_print_lock();

	fprintf(f, "%s %s %s ", r->name, dns_class_string(r->dnsclass), dns_type_string(r->dnstype));
	fprintf(f, "%u", r->ttl);

	switch (r->dnstype)
	{
		case ns_t_a:
			fprintf(f, " %s", inet_ntoa(r->data.A->addr));
			break;

		case ns_t_aaaa:
			memset(&s6, 0, sizeof(struct sockaddr_in6));
			s6.sin6_len = sizeof(struct sockaddr_in6);
			s6.sin6_family = AF_INET6;
			s6.sin6_addr = r->data.AAAA->addr;
			fprintf(f, " %s", inet_ntop(AF_INET6, (char *)(&s6) + INET_NTOP_AF_INET6_OFFSET, pbuf, 64));
			break;

		case ns_t_md:
		case ns_t_mf:
		case ns_t_cname:
		case ns_t_mb:
		case ns_t_mg:
		case ns_t_mr:
		case ns_t_ptr:
		case ns_t_ns:
			fprintf(f, " %s", r->data.CNAME->name);
			break;

		case ns_t_soa:
			fprintf(f, " %s %s %u %u %u %u %u",
				r->data.SOA->mname, r->data.SOA->rname,
				r->data.SOA->serial, r->data.SOA->refresh, r->data.SOA->retry,
				r->data.SOA->expire, r->data.SOA->minimum);
			break;

		case ns_t_wks:
			fprintf(f, " %s", inet_ntoa(r->data.WKS->addr));
			p = getprotobynumber(r->data.WKS->protocol);
			if (p != NULL)
			{
				fprintf(f, " %s", p->p_name);

				for (i = 0; i < r->data.WKS->maplength; i++)
				{
					if (r->data.WKS->map[i])
					{
						s = getservbyport(i, p->p_name);
						if (s == NULL) fprintf(f, " %u", i);
						else fprintf(f, " %s", s->s_name);
					}
				}
			}
			else fprintf(f, " UNKNOWN PROTOCOL %u", r->data.WKS->protocol);
			break;

		case ns_t_hinfo:
			fprintf(f, " %s %s", r->data.HINFO->cpu, r->data.HINFO->os);
			break;

		case ns_t_minfo:
			fprintf(f, " %s %s", r->data.MINFO->rmailbx, r->data.MINFO->emailbx);
			break;

		case ns_t_mx:
			fprintf(f, " %u %s", r->data.MX->preference, r->data.MX->name);
			break;

		case ns_t_txt:
			for (i = 0; i < r->data.TXT->string_count; i++)
			{
				fprintf(f, " \"%s\"", r->data.TXT->strings[i]);
			}
			break;

		case ns_t_rp:
			fprintf(f, " %s %s", r->data.RP->mailbox, r->data.RP->txtdname);
			break;

		case ns_t_afsdb:
			fprintf(f, " %u %s", r->data.AFSDB->subtype,
				r->data.AFSDB->hostname);
			break;

		case ns_t_x25:
			fprintf(f, " %s", r->data.X25->psdn_address);
			break;

		case ns_t_isdn:
			fprintf(f, " %s", r->data.ISDN->isdn_address);
			if (r->data.ISDN->subaddress != NULL)
				fprintf(f, " %s", r->data.ISDN->subaddress);
			break;

		case ns_t_rt:
			fprintf(f, " %hu %s", r->data.RT->preference,
				r->data.RT->intermediate);
			break;

		case ns_t_loc:
			fprintf(f, " %s", coord_ntoa(r->data.LOC->latitude, 1));
			fprintf(f, " %s", coord_ntoa(r->data.LOC->longitude, 0));
			fprintf(f, " %sm", alt_ntoa(r->data.LOC->altitude));
			fprintf(f, " %sm", precsize_ntoa(r->data.LOC->size));
			fprintf(f, " %sm", precsize_ntoa(r->data.LOC->horizontal_precision));
			fprintf(f, " %sm", precsize_ntoa(r->data.LOC->vertical_precision));
			break;

		case ns_t_srv:
			fprintf(f, " %hu %hu %hu %s",
				r->data.SRV->priority, r->data.SRV->weight, 
				r->data.SRV->port, r->data.SRV->target);
			break;

		case ns_t_null:
		default:
			len = r->data.DNSNULL->length;
			fprintf(f, " %u ", len);
			for (i = 0; i < len; i++)
			{
				x = r->data.DNSNULL->data[i];
				fprintf(f, "%x", x);
			}

			fprintf(f, " (");

			len = r->data.DNSNULL->length;
			for (i = 0; i < len; i++)
			{
				x = r->data.DNSNULL->data[i];
				if (isascii(x)) fprintf(f, "%c", x);
				else fprintf(f, " ");
			}
			fprintf(f, ")\n");

	}

	fprintf(f, "\n");

	if (lockit != 0) _dns_print_unlock();
}

void
dns_print_resource_record(const dns_resource_record_t *r, FILE *f)
{
	_dns_print_resource_record_lock(r, f, 1);
}

void
dns_print_reply(const dns_reply_t *r, FILE *f, uint16_t mask)
{
	uint16_t i;
	dns_header_t *h;
	char scratch[1024];
	uint32_t offset, iface;

	_dns_print_lock();

	if (r == NULL)
	{
		fprintf(f, "-nil-\n");
		_dns_print_unlock();
		return;
	}

	if (r->status != DNS_STATUS_OK)
	{
		if (r->status == DNS_STATUS_TIMEOUT)
			fprintf(f, "Timeout\n");
		else if (r->status == DNS_STATUS_SEND_FAILED)
			fprintf(f, "Send failed\n");
		else if (r->status == DNS_STATUS_RECEIVE_FAILED)
			fprintf(f, "Receive failed\n");
		else fprintf(f, "status %u\n", r->status);

		_dns_print_unlock();
		return;
	}

	h = r->header;

	if (mask & DNS_PRINT_XID)
	{
		fprintf(f, "Xid: %u\n", h->xid);
	}

	if (mask & DNS_PRINT_QR)
	{
		if ((h->flags & DNS_FLAGS_QR_MASK) == DNS_FLAGS_QR_QUERY)
			fprintf(f, "QR: Query\n");
		else
			fprintf(f, "QR: Reply\n");
	}

	if (mask & DNS_PRINT_SERVER)
	{
		if (r->server == NULL)
		{
			fprintf(f, "Server: -nil-\n");
		}
		else
		{
			offset = INET_NTOP_AF_INET_OFFSET;
			if (r->server->sa_family == AF_INET6) offset = INET_NTOP_AF_INET6_OFFSET;

			fprintf(f, "Server: %s", inet_ntop(r->server->sa_family, (char *)(r->server) + offset, scratch, 1024));
			if (r->server->sa_family == AF_INET)
			{
				memcpy(&iface, (((struct sockaddr_in *)(r->server))->sin_zero), 4);
				if (iface > 0) fprintf(f, "%%%s", if_indextoname(iface, scratch));
			}
			else if (r->server->sa_family == AF_INET6)
			{
				iface = ((struct sockaddr_in6 *)(r->server))->sin6_scope_id;
				if (iface > 0) fprintf(f, "%%%s", if_indextoname(iface, scratch));
			}
			fprintf(f, "\n");
		}
	}

	if (mask & DNS_PRINT_OPCODE)
	{
		fprintf(f, "Opcode: ");
		switch (h->flags & DNS_FLAGS_OPCODE_MASK)
		{
			case ns_o_query:  fprintf(f, "Standard\n"); break;
			case ns_o_iquery: fprintf(f, "Inverse\n"); break;
			case ns_o_status: fprintf(f, "Status\n"); break;
			case ns_o_notify: fprintf(f, "Notify\n"); break;
			case ns_o_update: fprintf(f, "Update\n"); break;
			default:
				fprintf(f, "Reserved (%hu)\n",
					(uint16_t)((h->flags & DNS_FLAGS_OPCODE_MASK) >> 11));
		}
	}

	if (mask & DNS_PRINT_AA)
	{
		if (h->flags & DNS_FLAGS_AA) fprintf(f, "AA: Authoritative\n");
		else fprintf(f, "AA: Non-Authoritative\n");
	}

	if (mask & DNS_PRINT_TC)
	{
		if (h->flags & DNS_FLAGS_TC) fprintf(f, "TC: Truncated\n");
		else fprintf(f, "TC: Non-Truncated\n");
	}

	if (mask & DNS_PRINT_RD)
	{
		if (h->flags & DNS_FLAGS_RD) fprintf(f, "RD: Recursion desired\n");
		else fprintf(f, "RD: No recursion desired\n");
	}

	if (mask & DNS_PRINT_RA)
	{
		if (h->flags & DNS_FLAGS_RA) fprintf(f, "RA: Recursion available\n");
		else fprintf(f, "RA: No recursion available \n");
	}

	if (mask & DNS_PRINT_RCODE)
	{
		fprintf(f, "Rcode: ");
		switch (h->flags & DNS_FLAGS_RCODE_MASK)
		{
			case ns_r_noerror:
				fprintf(f, "No error\n");
				break;
			case ns_r_formerr:
				fprintf(f, "Format error \n");
				break;
			case ns_r_servfail:
				fprintf(f, "Server failure\n");
				break;
			case ns_r_nxdomain:
				fprintf(f, "Name error \n");
				break;
			case ns_r_notimpl:
				fprintf(f, "Not implemented\n");
				break;
			case ns_r_refused:
				fprintf(f, "Refused\n");
				break;
			case ns_r_yxdomain:
				fprintf(f, "Name exists\n");
				break;
			case ns_r_yxrrset:
				fprintf(f, "RR Set exists\n");
				break;
			case ns_r_nxrrset:
				fprintf(f, "RR Set does not exist\n");
				break;
			case ns_r_notauth:
				fprintf(f, "Not authoritative\n");
				break;
			case ns_r_notzone:
				fprintf(f, "Record zone does not match section zone\n");
				break;
			case ns_r_badvers:
				fprintf(f, "Invalid EDNS version or TSIG signature\n");
				break;
			case ns_r_badkey:
				fprintf(f, "Invalid key\n");
				break;
			case ns_r_badtime:
				fprintf(f, "Invalid time\n");
				break;
			default:
				fprintf(f, "Reserved (%hu)\n",(uint16_t)(h->flags & DNS_FLAGS_RCODE_MASK));
		}
	}

	if (mask & DNS_PRINT_QUESTION)
	{
		fprintf(f, "Question (%hu):\n", h->qdcount);
		for (i = 0; i < h->qdcount; i++)
			_dns_print_question_lock(r->question[i], f, 0);
	}

	if (mask & DNS_PRINT_ANSWER)
	{
		fprintf(f, "Answer (%hu):\n", h->ancount);
		for (i = 0; i < h->ancount; i++)
			_dns_print_resource_record_lock(r->answer[i], f, 0);
	}

	if (mask & DNS_PRINT_AUTHORITY)
	{
		fprintf(f, "Authority (%hu):\n", h->nscount);
		for (i = 0; i < h->nscount; i++)
			_dns_print_resource_record_lock(r->authority[i], f, 0);
	}

	if (mask & DNS_PRINT_ADDITIONAL)
	{
		fprintf(f, "Additional records (%hu):\n", h->arcount);
		for (i = 0; i < h->arcount; i++)
			_dns_print_resource_record_lock(r->additional[i], f, 0);
	}

	_dns_print_unlock();
}

static void
_pdns_print_handle(pdns_handle_t *pdns, FILE *f)
{
	uint32_t i, offset;
	struct in_addr a;
	char scratch[1024];
	struct sockaddr *sa;
	res_state r;

	if (pdns == NULL)
	{
		fprintf(f, "-nil-\n");
		return;
	}

	if (pdns->name == NULL) fprintf(f, "Name: -nil-\n");
	else fprintf(f, "Name: %s\n", pdns->name);

	fprintf(f, "Flags:");
	if (pdns->flags == 0) fprintf(f, " None\n");
	else 
	{
		if (pdns->flags & DNS_FLAG_DEBUG) fprintf(f, " Debug");
		if (pdns->flags & DNS_FLAG_CHECK_RESOLVER_DIR) fprintf(f, " DirCheck");
		if (pdns->flags & DNS_FLAG_HAVE_IPV6_SERVER) fprintf(f, " IPv6");
		if (pdns->flags & DNS_FLAG_OK_TO_SKIP_AAAA) fprintf(f, " SkipAAAA");
		if (pdns->flags & DNS_FLAG_DEFAULT_RESOLVER) fprintf(f, " Default");
		fprintf(f, "\n");
	}

	r = pdns->res;
	if (r == NULL) return;

	if (r->defdname[0] != '\0') fprintf(f, "Domain: %s\n", r->defdname);
	fprintf(f, "Search Order: %d\n", pdns->search_order);
	fprintf(f, "Total Timeout: %d\n", pdns->total_timeout);
	fprintf(f, "Retry Timeout: %d\n", pdns->res->retrans);
	fprintf(f, "Retry Attempts: %d\n", pdns->res->retry);

	fprintf(f, "Server%s:\n", (r->nscount == 1) ? "" : "s");
	for (i = 0; i < r->nscount; i++)
	{
		sa = get_nsaddr(r, i);
		offset = INET_NTOP_AF_INET_OFFSET;
		if (sa->sa_family == AF_INET6) offset = INET_NTOP_AF_INET6_OFFSET;
		fprintf(f, "  %u: %s", i, inet_ntop(sa->sa_family, (char *)sa + offset, scratch, 1024));
		fprintf(f, "\n");
	}

	if (pdns->search_count > 0)
	{
		fprintf(f, "Search List:\n");
		for (i = 0; i < pdns->search_count; i++)
			fprintf(f, "  %u: %s\n", i, pdns->search_list[i]);
	}

	if (r->sort_list[0].addr.s_addr != 0)
	{
		fprintf(f, "Sortlist:\n");
		for (i = 0; (r->sort_list[i].addr.s_addr != 0); i++)
		{
			fprintf(f, "  %u: ", i);
			a.s_addr = r->sort_list[i].addr.s_addr;
			fprintf(f, "%s/", inet_ntoa(a));
			a.s_addr = r->sort_list[i].mask;
			fprintf(f, "%s\n", inet_ntoa(a));
		}
	}
}

static void
_sdns_print_handle(sdns_handle_t *sdns, FILE *f)
{
	int i;

	if (sdns == NULL)
	{
		fprintf(f, "-nil-\n");
		return;
	}

	for (i = 0; i < sdns->client_count; i++)
	{
		fprintf(f, "DNS client %d\n", i);
		_pdns_print_handle(sdns->client[i], f);
		fprintf(f, "\n");
	}

	fprintf(f, "resolver dir mod time = %u\n", sdns->modtime);
	fprintf(f, "resolver dir stat time = %u\n", sdns->stattime);
	fprintf(f, "resolver dir stat latency = %u\n", sdns->stat_latency);
}

void
dns_print_handle(dns_handle_t d, FILE *f)
{
	dns_private_handle_t *dns;

	_dns_print_lock();

	if (d == NULL)
	{
		fprintf(f, "-nil-\n");
		_dns_print_unlock();
		return;
	}

	dns = (dns_private_handle_t *)d;

	if (dns->handle_type == DNS_PRIVATE_HANDLE_TYPE_SUPER)
	{
		_sdns_print_handle(dns->sdns, f);
	}
	else
	{
		_pdns_print_handle(dns->pdns, f);
	}

	_dns_print_unlock();
}

void
dns_all_server_addrs(dns_handle_t d, struct sockaddr ***addrs, uint32_t *count)
{
	int i, j, k, n, found;
	dns_private_handle_t *dns;
	pdns_handle_t *pdns;
	struct sockaddr *sa;
	struct sockaddr_storage **l;
	res_state r;

	*addrs = NULL;
	*count = 0;
	l = NULL;
	n = 0;

	if (d == NULL) return;

	dns = (dns_private_handle_t *)d;

	if (dns->handle_type != DNS_PRIVATE_HANDLE_TYPE_SUPER) return;

	if (dns->sdns == NULL) return;

	/* Just to initialize / validate clients */
	i = dns_search_list_count(d);

	for (i = 0; i < dns->sdns->client_count; i++)
	{
		pdns = dns->sdns->client[i];
		if (pdns == NULL) continue;

		r = pdns->res;
		if (r == NULL) continue;

		for (j = 0; j < r->nscount; j++)
		{
			sa = get_nsaddr(r, j);
			found = 0;
			for (k = 0; (found == 0) && (k < n); k++)
			{
				if (memcmp(l[k], sa, sa->sa_len) == 0) found = 1;
			}
			if (found == 1) continue;

			if (n == 0)
			{
				l = (struct sockaddr_storage **)calloc(1, sizeof(struct sockaddr_storage *));
			}
			else
			{
				l = (struct sockaddr_storage **)reallocf(l, (n + 1) * sizeof(struct sockaddr_storage *));
			}

			if (l == NULL) return;

			l[n] = (struct sockaddr_storage *)calloc(1, sizeof(struct sockaddr_storage));
			if (l[n] == NULL) return;

			memset(l[n], 0, sizeof(struct sockaddr_storage));
			memcpy(l[n], sa, sa->sa_len);
			n++;
		}
	}

	*addrs = (struct sockaddr **)l;
	*count = n;
}

int
dns_res_once(struct sockaddr *server, struct timeval *timeout, int options, const char *name, int class, int type, u_char *res, int *reslen)
{
	res_state statp;
	int n, fromlen, status;
	struct sockaddr_storage from;
	u_char buf[MAXPACKET];

	if (server == NULL) return DNS_RES_STATUS_INVALID_ARGUMENT;
	if (name == NULL) return DNS_RES_STATUS_INVALID_ARGUMENT;
	if (res == NULL) return DNS_RES_STATUS_INVALID_ARGUMENT;
	if (reslen == NULL) return DNS_RES_STATUS_INVALID_ARGUMENT;

	fromlen = sizeof(struct sockaddr_storage);

	statp = res_state_new();
	statp->retry = 1;
	statp->options = options;
	statp->id = res_randomid();
	if (timeout == NULL) statp->retrans = 5;
	else statp->retrans = timeout->tv_sec;

	statp->ndots = 1;
	statp->_vcsock = -1;
	statp->nscount = 1;

	strcpy(statp->_u._ext.ext->nsuffix, "ip6.arpa");
	strcpy(statp->_u._ext.ext->nsuffix2, "ip6.int");
	strcpy(statp->_u._ext.ext->bsuffix, "ip6.arpa");

	if (server->sa_family == AF_INET6)
	{
		memcpy(&(statp->_u._ext.ext->nsaddrs[0]), server, sizeof(struct sockaddr_in6));
		statp->nsaddr_list[0].sin_family = 0;
	}
	else
	{
		memcpy(&(statp->_u._ext.ext->nsaddrs[0]), server, sizeof(struct sockaddr_in));
		memcpy(&(statp->nsaddr_list[0]), server, sizeof(struct sockaddr_in));
	}

	n = res_nmkquery(statp, ns_o_query, name, class, type, NULL, 0, NULL, buf, sizeof(buf));

	status = dns_res_send(statp, buf, n, res, reslen, (struct sockaddr *)&from, &fromlen);

	res_client_close(statp);

	return status;
}
