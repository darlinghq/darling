/*
 * Copyright (c) 2008-2011 Apple Inc.  All rights reserved.
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

#include <netdb.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <network/sa_compare.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <string.h>
#include <sys/param.h>
#include <notify.h>
#include <notify_keys.h>
#include <pthread.h>
#include <TargetConditionals.h>
#include "netdb_async.h"
#include "si_module.h"

#define SOCK_UNSPEC 0
#define IPPROTO_UNSPEC 0

#define IPV6_ADDR_LEN 16
#define IPV4_ADDR_LEN 4

#define WANT_NOTHING 0
#define WANT_A4_ONLY 1
#define WANT_A6_ONLY 2
#define WANT_A6_PLUS_MAPPED_A4 3
#define WANT_A6_OR_MAPPED_A4_IF_NO_A6 4

#define V6TO4_PREFIX_16 0x2002

static int net_config_token = -1;
static uint32_t net_v4_count = 0;
static uint32_t net_v6_count = 0;	// includes 6to4 addresses
static pthread_mutex_t net_config_mutex = PTHREAD_MUTEX_INITIALIZER;

// Libc SPI
int _inet_aton_check(const char *cp, struct in_addr *addr, int strict);

typedef struct {
	struct hostent host;
	int alias_count;
	int addr_count;
	uint64_t ttl;
} build_hostent_t;

__private_extern__ int
si_inet_config(uint32_t *inet4, uint32_t *inet6)
{
	int status, checkit;
	struct ifaddrs *ifa, *ifap;

	pthread_mutex_lock(&net_config_mutex);

	checkit = 1;

	if (net_config_token < 0)
	{
		status = notify_register_check(kNotifySCNetworkChange, &net_config_token);
		if (status != 0) net_config_token = -1;
	}

	if (net_config_token >= 0)
	{
		status = notify_check(net_config_token, &checkit);
		if (status != 0) checkit = 1;
	}

	status = 0;

	if (checkit != 0)
	{
		if (getifaddrs(&ifa) < 0)
		{
			status = -1;
		}
		else
		{
			net_v4_count = 0;
			net_v6_count = 0;

			for (ifap = ifa; ifap != NULL; ifap = ifap->ifa_next)
			{
				if (ifap->ifa_addr == NULL) continue;
				if ((ifap->ifa_flags & IFF_UP) == 0) continue;

				if (ifap->ifa_addr->sa_family == AF_INET)
				{
					net_v4_count++;
				}
				else if (ifap->ifa_addr->sa_family == AF_INET6)
				{
					net_v6_count++;
				}
			}
            freeifaddrs(ifa);
		}

	}

	if (inet4 != NULL) *inet4 = net_v4_count;
	if (inet6 != NULL) *inet6 = net_v6_count;

	pthread_mutex_unlock(&net_config_mutex);

	return status;
}

void
freeaddrinfo(struct addrinfo *a)
{
	struct addrinfo *next;

	while (a != NULL)
	{
		next = a->ai_next;
		if (a->ai_addr != NULL) free(a->ai_addr);
		if (a->ai_canonname != NULL) free(a->ai_canonname);
		free(a);
		a = next;
	}
}

const char *
gai_strerror(int32_t err)
{
	switch (err)
	{
		case EAI_ADDRFAMILY: return "Address family for nodename not supported";
		case EAI_AGAIN: return "Temporary failure in name resolution";
		case EAI_BADFLAGS:	return "Invalid value for ai_flags";
		case EAI_FAIL: return "Non-recoverable failure in name resolution";
		case EAI_FAMILY: return "ai_family not supported";
		case EAI_MEMORY: return "Memory allocation failure";
		case EAI_NODATA: return "No address associated with nodename";
		case EAI_NONAME: return "nodename nor servname provided, or not known";
		case EAI_SERVICE: return "servname not supported for ai_socktype";
		case EAI_SOCKTYPE: return "ai_socktype not supported";
		case EAI_SYSTEM: return "System error";
		case EAI_BADHINTS: return "Bad hints";
		case EAI_PROTOCOL: return "ai_protocol not supported";
		case EAI_OVERFLOW: return "argument buffer overflow";
	}

	return "Unknown error";
}

/*
 * getnameinfo
 *
 * We handle some "trival" cases locally.  If the caller passes
 * NI_NUMERICHOST (only), then this call turns into a getservbyport
 * to get the service name + inet_pton() to create a host string.
 * If the caller passes NI_NUMERICSERV (only), then we zero out the port
 * number, complete the getnameinfo, and use printf() to create a service
 * string.  If the caller specifies both NI_NUMERICHOST and NI_NUMERICSERV,
 * we inet_ntop() and printf() and return the results.
 */
si_item_t *
si_nameinfo(si_mod_t *si, const struct sockaddr *sa, int flags, const char *interface, uint32_t *err)
{
	si_item_t *out = NULL;
	const struct sockaddr *lookup_sa;
	struct sockaddr_in s4;
	struct in_addr a4;
	struct in6_addr a6;
	const uint64_t unused = 0;
	void *addr = NULL;
	char *host = NULL;
	char *serv = NULL;
	uint32_t ifnum = 0;
	uint16_t port = 0;

	int do_host_lookup = ((flags & NI_NUMERICHOST) == 0);
	int do_serv_lookup = ((flags & NI_NUMERICSERV) == 0);

	/* check input */
	if ((si == NULL) || (sa == NULL))
	{
		if (err != NULL) *err = SI_STATUS_EAI_FAIL;
		return NULL;
	}

	if (err != NULL) *err = SI_STATUS_NO_ERROR;

	lookup_sa = sa;

	if (sa->sa_family == AF_INET)
	{
		struct sockaddr_in *s4 = (struct sockaddr_in *)sa;
		memcpy(&a4, &s4->sin_addr, sizeof(a4));
		port = s4->sin_port;
		addr = &a4;
	}
	else if (sa->sa_family == AF_INET6)
	{
		struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)sa;
		memcpy(&a6, &s6->sin6_addr, sizeof(a6));
		port = s6->sin6_port;

		/* Look for scope id in IPv6 Link Local, Multicast Node Local, and Multicast Link Local */
		if (IN6_IS_ADDR_LINKLOCAL(&s6->sin6_addr) || IN6_IS_ADDR_MC_NODELOCAL(&s6->sin6_addr) || IN6_IS_ADDR_MC_LINKLOCAL(&s6->sin6_addr))
		{
			ifnum = ntohs(a6.__u6_addr.__u6_addr16[1]);
			if (ifnum == 0)
			{
				ifnum = s6->sin6_scope_id;
				a6.__u6_addr.__u6_addr16[1] = htons(ifnum);
			}

			if ((ifnum != s6->sin6_scope_id) && (s6->sin6_scope_id != 0))
			{
				if (err != NULL) *err = SI_STATUS_EAI_FAIL;
				return NULL;
			}
		}

		/* v4 mapped and compat addresses are converted to plain v4 */
		if (IN6_IS_ADDR_V4MAPPED(&s6->sin6_addr) || IN6_IS_ADDR_V4COMPAT(&s6->sin6_addr))
		{
			memcpy(&a4, &s6->sin6_addr.s6_addr[12], sizeof(a4));
			addr = &a4;
			memset(&s4, 0, sizeof(s4));
			s4.sin_len = sizeof(s4);
			s4.sin_family = AF_INET;
			s4.sin_port = port;
			memcpy(&s4.sin_addr, &a4, sizeof(s4.sin_addr));
			lookup_sa = (const struct sockaddr *)&s4;
		}
		else
		{
			addr = &a6;
		}
	}
	else
	{
		if (err != NULL) *err = SI_STATUS_EAI_FAMILY;
		return NULL;
	}

	if (do_host_lookup == 1)
	{
		si_item_t *item = si_host_byaddr(si, addr, lookup_sa->sa_family, interface, NULL);
		if (item != NULL)
		{
			struct hostent *h;
			h = (struct hostent *)((uintptr_t)item + sizeof(si_item_t));
			host = strdup(h->h_name);
			si_item_release(item);
			if (host == NULL)
			{
				if (err != NULL) *err = SI_STATUS_EAI_MEMORY;
				return NULL;
			}
		}
	}

	if ((do_serv_lookup == 1) && (port != 0))
	{
		si_item_t *item = si_service_byport(si, port, NULL);
		if (item != NULL)
		{
			struct servent *s;
			s = (struct servent *)((uintptr_t)item + sizeof(si_item_t));
			serv = strdup(s->s_name);
			si_item_release(item);
			if (serv == NULL)
			{
				free(host);
				if (err != NULL) *err = SI_STATUS_EAI_MEMORY;
				return NULL;
			}
		}
	}

	/*
	 * Return numeric host name for NI_NUMERICHOST or if lookup failed, but not
	 * if NI_NAMEREQD is specified (so that we later fail with EAI_NONAME).
	 */
	if ((host == NULL) && ((flags & NI_NAMEREQD) == 0))
	{
		char tmp[INET6_ADDRSTRLEN + 1 + IF_NAMESIZE + 1];
		tmp[0] = '\0';
		if (sa->sa_family == AF_INET)
		{
			char buf[INET_ADDRSTRLEN];
			if (inet_ntop(AF_INET, &a4, buf, sizeof(buf)) != 0)
			{
				host = strdup(buf);
			}
		}
		else if (sa->sa_family == AF_INET6)
		{
			char buf[INET6_ADDRSTRLEN];

			/* zero the embedded scope ID */
			if (ifnum != 0)
			{
				a6.__u6_addr.__u6_addr16[1] = 0;
			}

			if (inet_ntop(AF_INET6, &a6, buf, sizeof(buf)) != 0)
			{
				if (ifnum != 0)
				{
					char ifname[IF_NAMESIZE];
					if (if_indextoname(ifnum, ifname) != NULL)
					{
						asprintf(&host, "%s%%%s", buf, ifname);
					}
					else
					{
						/* ENXIO */
						if (err != NULL) *err = SI_STATUS_EAI_FAIL;
						return NULL;
					}
				}
				else
				{
					host = strdup(buf);
				}
			}
		}
	}

	/* Return numeric service name for NI_NUMERICSERV or if lookup failed. */
	if (serv == NULL)
	{
		asprintf(&serv, "%hu", ntohs(port));
	}

	if ((host == NULL) || (serv == NULL))
	{
		if (err != NULL)
		{
			if ((flags & NI_NAMEREQD) != 0)
			{
				*err = SI_STATUS_EAI_NONAME;
			}
			else
			{
				*err = SI_STATUS_EAI_MEMORY;
			}
		}
	}
	else
	{
		out = (si_item_t *)LI_ils_create("L4488ss", (unsigned long)si, CATEGORY_NAMEINFO, 1, unused, unused, host, serv);
	}

	free(host);
	free(serv);
	return out;
}

static int
_gai_numericserv(const char *serv, uint16_t *port)
{
	int numeric;
	char *endptr;
	long num;

	numeric = 0;

	if (serv == NULL)
	{
		if (port) *port = 0;
		numeric = 1;
	}
	else
	{
		num = strtol(serv, &endptr, 10);
		if ((serv[0] != '\0') && (*endptr == '\0') && (num >= 0) && (num <= UINT16_MAX))
		{
			numeric = 1;
			if (port != NULL) *port = (uint16_t)num;
		}
	}

	return numeric;
}

int
_gai_serv_to_port(const char *serv, uint32_t proto, uint16_t *port)
{
	si_item_t *item;
	struct servent *s;
	const char *protoname = NULL;

	if (_gai_numericserv(serv, port)) return 0;

	if (proto == IPPROTO_UDP) protoname = "udp";
	if (proto == IPPROTO_TCP) protoname = "tcp";

	item = si_service_byname(si_search(), serv, protoname);
	if (item == NULL) return -1;

	s = (struct servent *)((uintptr_t)item + sizeof(si_item_t));
	if (port) *port = ntohs(s->s_port);
	si_item_release(item);

	return 0;
}

si_item_t *
si_addrinfo_v4(si_mod_t *si, int32_t flags, int32_t sock, int32_t proto, uint16_t port, struct in_addr *addr, uint16_t iface, const char *cname)
{
	socket_data_t sockdata;
	struct sockaddr_in *sa;
	int32_t len, v32;
	uint64_t unused;

	unused = 0;
	len = sizeof(struct sockaddr_in);
	memset(&sockdata, 0, sizeof(socket_data_t));
	sa = (struct sockaddr_in *)&sockdata;

	sa->sin_len = len;
	sa->sin_family = AF_INET;
	sa->sin_port = htons(port);
	memcpy(&sa->sin_addr, addr, sizeof(sa->sin_addr));

	/* Kludge: Jam the interface number into sin_zero (4 bytes). */
	v32 = iface;
	memmove(sa->sin_zero, &v32, sizeof(uint32_t));

	return (si_item_t *)LI_ils_create("L448844444Ss", (unsigned long)si, CATEGORY_ADDRINFO, 1, unused, unused, flags, AF_INET, sock, proto, len, sockdata, cname);
}

si_item_t *
si_addrinfo_v4_mapped(si_mod_t *si, int32_t flags, int32_t sock, int32_t proto, uint16_t port, struct in_addr *addr, uint16_t iface, const char *cname)
{
	socket_data_t sockdata;
	struct sockaddr_in6 *sa;
	int32_t len;
	uint64_t unused;

	unused = 0;
	len = sizeof(struct sockaddr_in6);
	memset(&sockdata, 0, sizeof(socket_data_t));
	sa = (struct sockaddr_in6 *)&sockdata;

	sa->sin6_len = len;
	sa->sin6_family = AF_INET6;
	sa->sin6_port = htons(port);
	memset(&(sa->sin6_addr.__u6_addr.__u6_addr8[10]), 0xff, 2);
	memcpy(&(sa->sin6_addr.__u6_addr.__u6_addr8[12]), addr, sizeof(struct in_addr));

	/* sin6_scope_id is in host byte order */
	sa->sin6_scope_id = iface;

	return (si_item_t *)LI_ils_create("L448844444Ss", (unsigned long)si, CATEGORY_ADDRINFO, 1, unused, unused, flags, AF_INET6, sock, proto, len, sockdata, cname);
}


si_item_t *
si_addrinfo_v6(si_mod_t *si, int32_t flags, int32_t sock, int32_t proto, uint16_t port, struct in6_addr *addr, uint16_t iface, const char *cname)
{
	socket_data_t sockdata;
	struct sockaddr_in6 *sa;
	int32_t len;
	uint64_t unused;

	unused = 0;
	len = sizeof(struct sockaddr_in6);
	memset(&sockdata, 0, sizeof(socket_data_t));
	sa = (struct sockaddr_in6 *)&sockdata;

	sa->sin6_len = len;
	sa->sin6_family = AF_INET6;
	sa->sin6_port = htons(port);
	memcpy(&sa->sin6_addr, addr, sizeof(sa->sin6_addr));

	/* sin6_scope_id is in host byte order */
	sa->sin6_scope_id = iface;

	if (IN6_IS_ADDR_LINKLOCAL(&sa->sin6_addr))
	{
		/* check for embedded scopeid */
		uint16_t esid = ntohs(sa->sin6_addr.__u6_addr.__u6_addr16[1]);
		if (esid != 0)
		{
			sa->sin6_addr.__u6_addr.__u6_addr16[1] = 0;
			if (iface == 0) sa->sin6_scope_id = esid;
		}
	}

	return (si_item_t *)LI_ils_create("L448844444Ss", (unsigned long)si, CATEGORY_ADDRINFO, 1, unused, unused, flags, AF_INET6, sock, proto, len, sockdata, cname);
}

si_list_t *
si_addrinfo_list(si_mod_t *si, uint32_t flags, int socktype, int proto, struct in_addr *a4, struct in6_addr *a6, int port, int scopeid, const char *cname4, const char *cname6)
{
	int do_map = 0;
	si_item_t *item = NULL;
	si_list_t *out4 = NULL, *out6 = NULL;

	if ((flags & AI_V4MAPPED) && ((flags & AI_ALL) || (a6 == NULL))) do_map = 1;

	if (a6 != NULL)
	{
		if ((proto == IPPROTO_UNSPEC) || (proto == IPPROTO_UDP))
		{
			item = si_addrinfo_v6(si, 0, SOCK_DGRAM, IPPROTO_UDP, port, a6, scopeid, cname6);
			out6 = si_list_add(out6, item);
			si_item_release(item);
		}

		if ((proto == IPPROTO_UNSPEC) || (proto == IPPROTO_TCP))
		{
			item = si_addrinfo_v6(si, 0, SOCK_STREAM, IPPROTO_TCP, port, a6, scopeid, cname6);
			out6 = si_list_add(out6, item);
			si_item_release(item);
		}

		if (proto == IPPROTO_ICMPV6)
		{
			item = si_addrinfo_v6(si, 0, SOCK_RAW, IPPROTO_ICMPV6, port, a6, scopeid, cname6);
			out6 = si_list_add(out6, item);
			si_item_release(item);
		}
	}

	if (a4 != NULL)
	{
		if ((proto == IPPROTO_UNSPEC) || (proto == IPPROTO_UDP))
		{
			if (do_map == 0)
			{
				item = si_addrinfo_v4(si, 0, SOCK_DGRAM, IPPROTO_UDP, port, a4, 0, cname4);
				out4 = si_list_add(out4, item);
			}
			else
			{
				item = si_addrinfo_v4_mapped(si, 0, SOCK_DGRAM, IPPROTO_UDP, port, a4, 0, cname4);
				out6 = si_list_add(out6, item);
			}

			si_item_release(item);
		}

		if ((proto == IPPROTO_UNSPEC) || (proto == IPPROTO_TCP))
		{
			if (do_map == 0)
			{
				item = si_addrinfo_v4(si, 0, SOCK_STREAM, IPPROTO_TCP, port, a4, 0, cname4);
				out4 = si_list_add(out4, item);
			}
			else
			{
				item = si_addrinfo_v4_mapped(si, 0, SOCK_STREAM, IPPROTO_TCP, port, a4, 0, cname4);
				out6 = si_list_add(out6, item);
			}

			si_item_release(item);
		}

		if (proto == IPPROTO_ICMP)
		{
			if (do_map == 0)
			{
				item = si_addrinfo_v4(si, 0, SOCK_RAW, IPPROTO_ICMP, port, a4, 0, cname4);
				out4 = si_list_add(out4, item);
			}
			else
			{
				item = si_addrinfo_v4_mapped(si, 0, SOCK_RAW, IPPROTO_ICMP, port, a4, 0, cname4);
				out6 = si_list_add(out6, item);
			}

			si_item_release(item);
		}
	}

	out6 = si_list_concat(out6, out4);
	si_list_release(out4);

	return out6;
}

/*
 * _gai_numerichost
 * Determines whether the given host name is a numeric IPv4 or IPv6 address,
 * based on the address family input value.  If the input addres family is
 * unspecified, a more specific value will be provided on output if possible.
 * Returns 1 if host name is numeric or 0 if not, or -1 on error.
 */
static int
_gai_numerichost(const char* nodename, uint32_t *family, int flags, struct in_addr *a4, struct in6_addr *a6, int *scope)
{
	int numerichost, passive;

	numerichost = 0;

	if (nodename == NULL)
	{
		/* return loopback or passive addresses */
		passive = (flags & AI_PASSIVE);

		if (((*family == AF_UNSPEC) || (*family == AF_INET)) || ((*family == AF_INET6) && (flags & AI_V4MAPPED) && (flags & AI_ALL)))
		{
			if (passive) a4->s_addr = 0;
			else a4->s_addr = htonl(INADDR_LOOPBACK);
		}

		if ((*family == AF_UNSPEC) || (*family == AF_INET6))
		{
			memset(a6, 0, sizeof(*a6));
			if (!passive) a6->__u6_addr.__u6_addr32[3] = htonl(1);
		}

		numerichost = 1;
	}
	else
	{
		/*
		 * numeric IPv4 host valid for AF_UNSPEC and AF_INET
		 * also valid for AF_INET6 with AI_V4MAPPED
		 */
		numerichost = inet_pton(AF_INET, nodename, a4);
		if (numerichost == 0)
		{
			/* inet_pton doesn't allow "a", "a.b", or "a.b.c" forms, so we re-check */
			numerichost = _inet_aton_check(nodename, a4, 1);
		}

		if (numerichost == 1)
		{
			if (*family == AF_UNSPEC)
			{
				*family = AF_INET;
			}
			else if (*family == AF_INET6)
			{
				if (flags & AI_V4MAPPED)
				{
					memset(a6, 0, sizeof(struct in6_addr));
					memset(&(a6->__u6_addr.__u6_addr8[10]), 0xff, 2);
					memcpy(&(a6->__u6_addr.__u6_addr8[12]), a4, sizeof(struct in_addr));
				}
				else
				{
					numerichost = -1;
				}
			}

			return numerichost;
		}

		/* numeric IPv6 host valid for AF_UNSPEC and AF_INET6 */
		numerichost = inet_pton(AF_INET6, nodename, a6);
		if (numerichost == 1)
		{
			/* check for scope/zone id */
			char *p = strrchr(nodename, SCOPE_DELIMITER);
			if (p != NULL)
			{
				int i, d;
				char *x;
				
				p++;
				d = 1;
				for (x = p; (*x != '\0') && (d == 1); x++)
				{
					i = *x;
					d = isdigit(i);
				}
				
				if (d == 1) *scope = atoi(p);
				else *scope = if_nametoindex(p);
			}

			if (*family == AF_UNSPEC) *family = AF_INET6;
			else if (*family == AF_INET) numerichost = -1;

			return numerichost;
		}
	}

	return numerichost;
}

/* si_addrinfo_list_from_hostent
 * Returns an addrinfo list from IPv4 and IPv6 hostent entries
 */
si_list_t *
si_addrinfo_list_from_hostent(si_mod_t *si, uint32_t flags, uint32_t socktype, uint32_t proto, uint16_t port, uint16_t scope, const struct hostent *h4, const struct hostent *h6)
{
	int i;
	si_list_t *out = NULL;
	si_list_t *list;

	if ((h6 != NULL) && (h6->h_addr_list != NULL))
	{
		for (i = 0; h6->h_addr_list[i] != NULL; i++)
		{
			struct in6_addr a6;
			memcpy(&a6, h6->h_addr_list[i], h6->h_length);
			list = si_addrinfo_list(si, flags, socktype, proto, NULL, &a6, port, scope, NULL, h6->h_name);
			out = si_list_concat(out, list);
			si_list_release(list);
		}
	}

	if ((h4 != NULL) && (h4->h_addr_list != NULL))
	{
		for (i = 0; h4->h_addr_list[i] != NULL; i++)
		{
			struct in_addr a4;
			memcpy(&a4, h4->h_addr_list[i], h4->h_length);
			list = si_addrinfo_list(si, flags, socktype, proto, &a4, NULL, port, 0, h4->h_name, NULL);
			out = si_list_concat(out, list);
			si_list_release(list);
		}
	}

	return out;
}

int
_gai_addr_sort(const void *a, const void *b)
{
	si_item_t **item_a, **item_b;
	si_addrinfo_t *p, *q;
	struct sockaddr *sp, *sq;
	
	item_a = (si_item_t **)a;
	item_b = (si_item_t **)b;

	p = (si_addrinfo_t *)((uintptr_t)*item_a + sizeof(si_item_t));
	q = (si_addrinfo_t *)((uintptr_t)*item_b + sizeof(si_item_t));

	sp = (struct sockaddr *)p->ai_addr.x;
	sq = (struct sockaddr *)q->ai_addr.x;

	/*
	 * sa_dst_compare(A,B) returns -1 if A is less desirable than B,
	 * 0 if they are equally desirable, and 1 if A is more desirable.
	 * qsort() expects the inverse, so we swap sp and sq.
	 */
	return sa_dst_compare(sq, sp, 0);
}

static si_list_t *
_gai_sort_list(si_list_t *in, uint32_t flags)
{
	si_list_t *out;
	int filter_mapped;
	uint32_t i;
	uint32_t v4mapped_count = 0;
	uint32_t v6_count = 0;
	si_addrinfo_t *a;

	if (in == NULL) return NULL;

	for (i = 0; i < in->count; i++)
	{
		a = (si_addrinfo_t *)((uintptr_t)in->entry[i] + sizeof(si_item_t));
		if (a->ai_family == AF_INET6)
		{
			struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)a->ai_addr.x;
			if (IN6_IS_ADDR_V4MAPPED(&(s6->sin6_addr))) v4mapped_count++;
			else v6_count++;
		}
	}

	filter_mapped = 1;
	if ((flags & AI_V4MAPPED) && ((v6_count == 0) || (flags & AI_ALL))) filter_mapped = 0;

	out = in;

	if ((filter_mapped == 1) && (v4mapped_count > 0))
	{
		i = in->count - v4mapped_count;
		if (i == 0) return NULL;

		out = (si_list_t *)calloc(1, sizeof(si_list_t));
		if (out == NULL) return in;

		out->count = i;
		out->refcount = in->refcount;

		out->entry = (si_item_t **)calloc(out->count, sizeof(si_item_t *));
		if (out->entry == NULL)
		{
			free(out);
			return in;
		}

		out->curr = 0;

		for (i = 0; i < in->count; i++)
		{
			a = (si_addrinfo_t *)((uintptr_t)in->entry[i] + sizeof(si_item_t));
			if (a->ai_family == AF_INET6)
			{
				struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)a->ai_addr.x;
				if (IN6_IS_ADDR_V4MAPPED(&(s6->sin6_addr)))
				{
					si_item_release(in->entry[i]);
					continue;
				}
			}

			out->entry[out->curr++] = in->entry[i];
		}

		out->curr = 0;

		free(in->entry);
		free(in);
	}

	qsort(&out->entry[0], out->count, sizeof(si_item_t *), _gai_addr_sort);
	return out;
}

/* _gai_simple
 * Simple lookup via gethostbyname2(3) mechanism.
 */
si_list_t *
_gai_simple(si_mod_t *si, const void *nodeptr, const void *servptr, uint32_t family, uint32_t socktype, uint32_t proto, uint32_t flags, const char *interface, uint32_t *err)
{
	si_item_t *h4_item = NULL, *h6_item = NULL;
	struct hostent *h4 = NULL, *h6 = NULL;
	si_list_t *out = NULL;
	uint16_t port;

	if ((flags & AI_NUMERICSERV) != 0)
	{
		port = *(uint16_t*)servptr;
	}
	else
	{
		if (_gai_serv_to_port(servptr, proto, &port) != 0)
		{
			if (err) *err = SI_STATUS_EAI_NONAME;
			return NULL;
		}
	}

	if ((flags & AI_NUMERICHOST) != 0)
	{
		if (family == AF_INET)
		{
			h4_item = si_host_byaddr(si, nodeptr, AF_INET, interface, NULL);
		}
		else if (family == AF_INET6)
		{
			h6_item = si_host_byaddr(si, nodeptr, AF_INET6, interface, NULL);
		}
	}
	else
	{
		if ((family == AF_INET) || (family == AF_UNSPEC))
		{
			h4_item = si_host_byname(si, nodeptr, AF_INET, interface, NULL);
		}

		if ((family == AF_INET6) || (family == AF_UNSPEC))
		{
			h6_item = si_host_byname(si, nodeptr, AF_INET6, interface, NULL);
		}
	}

	if (h4_item != NULL)
	{
		h4 = (struct hostent *)((uintptr_t)h4_item + sizeof(si_item_t));
	}

	if (h6_item != NULL)
	{
		h6 = (struct hostent *)((uintptr_t)h6_item + sizeof(si_item_t));
	}

	out = si_addrinfo_list_from_hostent(si, flags, socktype, proto, port, 0, h4, h6);
	si_item_release(h4_item);
	si_item_release(h6_item);

	return _gai_sort_list(out, flags);
}

si_list_t *
si_srv_byname(si_mod_t *si, const char *qname, const char *interface, uint32_t *err)
{
	if (si == NULL) return 0;
	if (si->vtable->sim_srv_byname == NULL) return 0;

	return si->vtable->sim_srv_byname(si, qname, interface, err);
}

int
si_wants_addrinfo(si_mod_t *si)
{
	if (si == NULL) return 0;
	if (si->vtable->sim_addrinfo == NULL) return 0;
	if (si->vtable->sim_wants_addrinfo == NULL) return 0;

	return si->vtable->sim_wants_addrinfo(si);
}

static si_list_t *
_gai_srv(si_mod_t *si, const char *node, const char *serv, uint32_t family, uint32_t socktype, uint32_t proto, uint32_t flags, const char *interface, uint32_t *err)
{
	int i;
	char *qname;
	si_srv_t *srv;
	si_item_t *item;

	si_list_t *list = NULL;
	si_list_t *result = NULL;

	/* Minimum SRV priority is zero. Start below that. */
	int lastprio = -1;
	int currprio;

	if (node == NULL || serv == NULL) return NULL;

	asprintf(&qname, "%s.%s", serv, node);
	list = si_srv_byname(si, qname, interface, err);
	free(qname);

	/* Iterate the SRV records starting at lowest priority and attempt to
	 * lookup the target host name. Returns the first successful lookup.
	 * It's an O(n^2) algorithm but data sets are small (less than 100) and
	 * sorting overhead is dwarfed by network I/O for each element.
	 */
	while (list != NULL && result == NULL)
	{
		/* Find the next lowest priority level. */
		/* Maximum SRV priority is UINT16_MAX. Start above that. */
		currprio = INT_MAX;

		for (i = 0; i < list->count; ++i)
		{
			item = list->entry[i];
			srv = (si_srv_t *)((uintptr_t)item + sizeof(si_item_t));

			if (srv->priority > lastprio && srv->priority < currprio)
			{
				currprio = srv->priority;
			}
		}

		if (currprio == INT_MAX)
		{
			/* All priorities have been evaluated. Done. */
			break;
		}
		else
		{
			lastprio = currprio;
		}

		/* Lookup hosts at the current priority level. Return first match. */
		for (i = 0; i < list->count; ++i)
		{
			item = list->entry[i];
			srv = (si_srv_t *)((uintptr_t)item + sizeof(si_item_t));

			if (srv->priority == currprio)
			{
				/* So that _gai_simple expects an integer service. */
				flags |= AI_NUMERICSERV;

				result = _gai_simple(si, srv->target, &srv->port, family, socktype, proto, flags, interface, err);
				if (result)
				{
					break;
				}
			}
		}
	}

	if (list != NULL)
	{
		si_list_release(list);
	}

	return result;
}

si_list_t *
si_addrinfo(si_mod_t *si, const char *node, const char *serv, uint32_t family, uint32_t socktype, uint32_t proto, uint32_t flags, const char *interface, uint32_t *err)
{
	int numerichost, numericserv = 0;
	int scope = 0;
	const void *nodeptr = NULL, *servptr = NULL;
	uint16_t port;
	struct in_addr a4, *p4;
	struct in6_addr a6, *p6;
	const char *cname;
	si_list_t *out;

	if (err != NULL) *err = SI_STATUS_NO_ERROR;

	if (si == NULL)
	{
		if (err != NULL) *err = SI_STATUS_EAI_FAIL;
		return NULL;
	}

	/* treat empty strings as NULL */
	if ((node != NULL) && (node[0] == '\0')) node = NULL;
	if ((serv != NULL) && (serv[0] == '\0')) serv = NULL;

	/* make sure we have a query */
	if ((node == NULL) && (serv == NULL))
	{
		if (err != NULL) *err = SI_STATUS_EAI_NONAME;
		return NULL;
	}

	/* verify family is supported */
	switch (family)
	{
		case AF_UNSPEC:
		case AF_INET:
		case AF_INET6:
			break;
		default:
			if (err != NULL) *err = SI_STATUS_EAI_FAMILY;
			return NULL;
	}

	/* verify socket type is supported */
	switch (socktype)
	{
		case SOCK_UNSPEC:
		case SOCK_RAW:
		case SOCK_DGRAM:
		case SOCK_STREAM:
			break;
		default:
			if (err != NULL) *err = SI_STATUS_EAI_BADHINTS;
			return NULL;
	}

	/* verify protocol is supported */
	switch (proto)
	{
		case IPPROTO_UNSPEC:
		case IPPROTO_UDP:
		case IPPROTO_TCP:
		case IPPROTO_ICMP:
		case IPPROTO_ICMPV6:
			break;
		default:
			if (err != NULL) *err = SI_STATUS_EAI_BADHINTS;
			return NULL;
	}

	/* verify socket type compatible with protocol */
	if (((socktype == SOCK_DGRAM) && (proto != IPPROTO_UNSPEC) && (proto != IPPROTO_UDP)) || ((socktype == SOCK_STREAM) && (proto != IPPROTO_UNSPEC) && (proto != IPPROTO_TCP)))
	{
		if (err != NULL) *err = SI_STATUS_EAI_BADHINTS;
		return NULL;
	}

	/* check AI_V4MAPPED and AI_ALL */
	if (family != AF_INET6)
	{
		/* unset AI_V4MAPPED and AI_ALL unless family is AF_INET6 */
		flags &= ~(AI_V4MAPPED | AI_ALL);
	}
	else if ((flags & AI_V4MAPPED) == 0)
	{
		/* unset AI_ALL unless family is AF_INET6 and AI_V4MAPPED is set */
		flags &= ~AI_ALL;
	}

	memset(&a4, 0, sizeof(struct in_addr));
	memset(&a6, 0, sizeof(struct in6_addr));

	/* determine the protocol if possible */
	if ((proto == IPPROTO_UNSPEC) && (socktype == SOCK_DGRAM)) proto = IPPROTO_UDP;
	if ((proto == IPPROTO_UNSPEC) && (socktype == SOCK_STREAM)) proto = IPPROTO_TCP;

	port = 0;

	if ((flags & AI_SRV) != 0)
	{
		/* AI_SRV SPI */
		out = _gai_srv(si, node, serv, family, socktype, proto, flags, interface, err);
		return _gai_sort_list(out, flags);
	}
	else
	{
		/* Usual service lookup */
		numericserv = _gai_numericserv(serv, &port);
	}

	if ((flags & AI_NUMERICSERV) && (numericserv == 0))
	{
		/* FreeBSD returns SI_STATUS_EAI_SERVICE */
		if (err != NULL) *err = SI_STATUS_EAI_NONAME;
		return NULL;
	}

	if ((serv != NULL) && (strcmp(serv, "0") != 0))
	{
		if (numericserv == 1)
		{
			flags |= AI_NUMERICSERV;
			servptr = &port;
		}
		else
		{
			servptr = serv;
		}
	}

	numerichost = _gai_numerichost(node, &family, flags, &a4, &a6, &scope);
	if ((numerichost == -1) || ((flags & AI_NUMERICHOST) && (numerichost == 0)))
	{
		if (err != NULL) *err = SI_STATUS_EAI_NONAME;
		return NULL;
	}

	if (numerichost == 1)
	{
		flags |= AI_NUMERICHOST;
		if (family == AF_INET)
		{
			nodeptr = &a4;
		}
		else if (family == AF_INET6)
		{
			nodeptr = &a6;
		}
	}
	else
	{
		nodeptr = node;
	}

	if ((numerichost == 1) && (numericserv == 0))
	{
		/* only service lookup needed.  convert to port and perform a trivial getaddrinfo */
		if (_gai_serv_to_port(serv, proto, &port) != 0)
		{
			if (err != NULL) *err = SI_STATUS_EAI_NONAME;
			return NULL;
		}
		else
		{
			flags |= AI_NUMERICSERV;
			servptr = &port;
			numericserv = 1;
		}
	}

	if ((numerichost == 1) && (numericserv == 1))
	{
		p4 = &a4;
		p6 = &a6;
		cname = NULL;

		if (family == AF_INET) p6 = NULL;
		if (family == AF_INET6) p4 = NULL;
		if (node == NULL) cname = "localhost";

		/* allow nodename to be a mapped IPv4 address, e.g. "::ffff:10.0.0.1" */
		if (p6 != NULL) flags |= AI_V4MAPPED;

		/* handle trivial questions */
		out = si_addrinfo_list(si, flags, socktype, proto, p4, p6, port, scope, cname, cname);
		return _gai_sort_list(out, flags);
	}
	else if (si_wants_addrinfo(si))
	{
		/* or let the current module handle the host lookups intelligently */
		out = si->vtable->sim_addrinfo(si, nodeptr, servptr, family, socktype, proto, flags, interface, err);
		return _gai_sort_list(out, flags);
	}

	/* fall back to a default path */
	out = _gai_simple(si, nodeptr, servptr, family, socktype, proto, flags, interface, err);
	return _gai_sort_list(out, flags);
}

static struct addrinfo *
si_item_to_addrinfo(si_item_t *item)
{
	si_addrinfo_t *a;
	struct addrinfo *out;

	if (item == NULL) return NULL;

	a = (si_addrinfo_t *)((uintptr_t)item + sizeof(si_item_t));

	out = (struct addrinfo *)calloc(1, sizeof(struct addrinfo));
	if (out == NULL) return NULL;

	out->ai_flags = a->ai_flags;
	out->ai_family = a->ai_family;
	out->ai_socktype = a->ai_socktype;
	out->ai_protocol = a->ai_protocol;
	out->ai_addrlen = a->ai_addrlen;

	out->ai_addr = (struct sockaddr *)calloc(1, out->ai_addrlen);
	if (out->ai_addr == NULL)
	{
		free(out);
		return NULL;
	}

	memcpy(out->ai_addr, a->ai_addr.x, out->ai_addrlen);

	if (a->ai_canonname != NULL)
	{
		out->ai_canonname = strdup(a->ai_canonname);
		if (out->ai_canonname == NULL)
		{
			free(out);
			return NULL;
		}
	}

	return out;
}

struct addrinfo *
si_list_to_addrinfo(si_list_t *list)
{
	struct addrinfo *tail, *out;
	int i;

	if (list == NULL) return NULL;
	if (list->count == 0) return NULL;

	i = list->count - 1;

	out = si_item_to_addrinfo(list->entry[i]);
	tail = out;

	for (i--; i >= 0; i--)
	{
		out = si_item_to_addrinfo(list->entry[i]);
		if (out == NULL)
		{
			freeaddrinfo(tail);
			return NULL;
		}

		out->ai_next = tail;
		tail = out;
	}

	return out;
}

/* getipnodebyname */

static si_item_t *
make_hostent(si_mod_t *si, const char *name, struct in_addr addr)
{
	char *addrs[2];
	char *aliases[1];
	uint64_t unused;

	if (name == NULL) return NULL;

	unused = 0;

	addrs[0] = (char *)&(addr.s_addr);
	addrs[1] = NULL;
	aliases[0] = NULL;

	return (si_item_t *)LI_ils_create("L4488s*44a", (unsigned long)si, CATEGORY_HOST_IPV4, 1, unused, unused, name, aliases, AF_INET, IPV4_ADDR_LEN, addrs);
}

static si_item_t *
make_hostent6(si_mod_t *si, const char *name, struct in6_addr addr)
{
	char *addrs[2];
	char *aliases[1];
	uint64_t unused;

	if (name == NULL) return NULL;

	unused = 0;

	addrs[0] = (char *)&(addr.__u6_addr.__u6_addr32[0]);
	addrs[1] = NULL;
	aliases[0] = NULL;

	return (si_item_t *)LI_ils_create("L4488s*44c", (unsigned long)si, CATEGORY_HOST_IPV6, 1, unused, unused, name, aliases, AF_INET6, IPV6_ADDR_LEN, addrs);
}

static char *
lower_case(const char *s)
{
	int i;
	char *t;

	if (s == NULL) return NULL;

	t = malloc(strlen(s) + 1);

	for (i = 0; s[i] != '\0'; i++) 
	{
		if ((s[i] >= 'A') && (s[i] <= 'Z')) t[i] = s[i] + 32;
		else t[i] = s[i];
	}

	t[i] = '\0';

	return t;
}

static int
merge_alias(const char *name, build_hostent_t *h)
{
	int i;

	if (name == NULL) return 0;
	if (h == NULL) return 0;

	if ((h->host.h_name != NULL) && (string_equal(name, h->host.h_name))) return 0;

	for (i = 0; i < h->alias_count; i++)
	{
		if (string_equal(name, h->host.h_aliases[i])) return 0;
	}

	if (h->alias_count == 0) h->host.h_aliases = (char **)calloc(2, sizeof(char *));
	else h->host.h_aliases = (char **)reallocf(h->host.h_aliases, (h->alias_count + 2) * sizeof(char *));

	if (h->host.h_aliases == NULL)
	{
		h->alias_count = 0;
		return -1;
	}

	h->host.h_aliases[h->alias_count] = lower_case(name);
	h->alias_count++;
	h->host.h_aliases[h->alias_count] = NULL;

	return 0;
}

static int
append_addr(const char *addr, uint32_t len, build_hostent_t *h)
{
	if (addr == NULL) return 0;
	if (h == NULL) return 0;

	if (h->addr_count == 0) h->host.h_addr_list = (char **)calloc(2, sizeof(char *));
	else h->host.h_addr_list = (char **)reallocf(h->host.h_addr_list, (h->addr_count + 2) * sizeof(char *));

	if (h->host.h_addr_list == NULL)
	{
		h->addr_count = 0;
		return -1;
	}

	h->host.h_addr_list[h->addr_count] = malloc(len);
	if (h->host.h_addr_list[h->addr_count] == NULL) return -1;

	memcpy(h->host.h_addr_list[h->addr_count], addr, len);
	h->addr_count++;
	h->host.h_addr_list[h->addr_count] = NULL;

	return 0;
}

static void
free_build_hostent(build_hostent_t *h)
{
	uint32_t i;
	char **aliases;

	if (h == NULL) return;

	if (h->host.h_name != NULL) free(h->host.h_name);
	h->host.h_name = NULL;

	aliases = h->host.h_aliases;
	if (aliases != NULL)
	{
		while (*aliases != NULL) free(*aliases++);
		free(h->host.h_aliases);
	}

	h->host.h_aliases = NULL;

	if (h->host.h_addr_list != NULL)
	{
		for (i = 0; h->host.h_addr_list[i] != NULL; i++) free(h->host.h_addr_list[i]);
		free(h->host.h_addr_list);
	}

	h->host.h_addr_list = NULL;
	free(h);
}

si_item_t *
si_ipnode_byname(si_mod_t *si, const char *name, int family, int flags, const char *interface, uint32_t *err)
{
	int i, status, want;
	uint32_t if4, if6;
	struct in_addr addr4;
	struct in6_addr addr6;
	si_item_t *item4, *item6;
	build_hostent_t *out;
	struct hostent *h;
	uint64_t unused;

	memset(&addr4, 0, sizeof(struct in_addr));
	memset(&addr6, 0, sizeof(struct in6_addr));

	if (err != NULL) *err = 0;

	if (family == AF_INET)
	{
		status = inet_aton(name, &addr4);
		if (status == 1)
		{
			/* create a host entry */
			item4 = make_hostent(si, name, addr4);
			if (item4 == NULL)
			{
				if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
				return NULL;
			}

			return item4;
		}
	}
	else if (family == AF_INET6)
	{
		status = inet_pton(family, name, &addr6);
		if (status == 1)
		{
			/* create a host entry */
			item6 = make_hostent6(si, name, addr6);
			if (item6 == NULL)
			{
				if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
				return NULL;
			}

			return item6;
		}

		status = inet_aton(name, &addr4);
		if (status == 1)
		{
			if (!(flags & (AI_V4MAPPED | AI_V4MAPPED_CFG)))
			{
				if (err != NULL) *err = SI_STATUS_H_ERRNO_HOST_NOT_FOUND;
				return NULL;
			}

			addr6.__u6_addr.__u6_addr32[0] = 0x00000000;
			addr6.__u6_addr.__u6_addr32[1] = 0x00000000;
			addr6.__u6_addr.__u6_addr32[2] = htonl(0x0000ffff);
			memmove(&(addr6.__u6_addr.__u6_addr32[3]), &(addr4.s_addr), IPV4_ADDR_LEN);

			/* create a host entry */
			item6 = make_hostent6(si, name, addr6);
			if (item6 == NULL)
			{
				if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
				return NULL;
			}

			return item6;
		}
	}
	else
	{
		if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
		return NULL;
	}

	/*
	 * IF AI_ADDRCONFIG is set, we need to know what interface flavors we really have.
	 */

	if4 = 0;
	if6 = 0;

	if (flags & AI_ADDRCONFIG)
	{
		if (si_inet_config(&if4, &if6) < 0)
		{
			if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
			return NULL;
		}

		/* Bail out if there are no interfaces */
		if ((if4 == 0) && (if6 == 0))
		{
			if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
			return NULL;
		}
	}

	/*
	 * Figure out what we want.
	 * If user asked for AF_INET, we only want V4 addresses.
	 */
	want = WANT_A4_ONLY;

	if (family == AF_INET)
	{
		if ((flags & AI_ADDRCONFIG) && (if4 == 0))
		{
			if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
			return NULL;
		}
	}
	else if (family == AF_INET6)
	{
		/* family == AF_INET6 */
		want = WANT_A6_ONLY;

		if (flags & (AI_V4MAPPED | AI_V4MAPPED_CFG))
		{
			if (flags & AI_ALL)
			{
				want = WANT_A6_PLUS_MAPPED_A4;
			}
			else
			{
				want = WANT_A6_OR_MAPPED_A4_IF_NO_A6;
			}
		}
		else
		{
			if ((flags & AI_ADDRCONFIG) && (if6 == 0)) 
			{
				if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
				return NULL;
			}
		}
	}
	else
	{
		if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
		return NULL;
	}

	item6 = NULL;
	item4 = NULL;

	/* fetch IPv6 data if required */
	if ((want == WANT_A6_ONLY) || (want == WANT_A6_OR_MAPPED_A4_IF_NO_A6) || (want == WANT_A6_PLUS_MAPPED_A4))
	{
		item6 = si_host_byname(si, name, AF_INET6, interface, (uint32_t *)err);
	}

	/* fetch IPv4 data if required */
	if ((want == WANT_A4_ONLY) || (want == WANT_A6_PLUS_MAPPED_A4) || ((want == WANT_A6_OR_MAPPED_A4_IF_NO_A6) && (item6 == NULL)))
	{
		item4 = si_host_byname(si, name, AF_INET, interface, (uint32_t *)err);
	}

	if (want == WANT_A4_ONLY)
	{
		si_item_release(item6);
		if ((item4 == NULL) && (err != NULL)) *err = SI_STATUS_H_ERRNO_HOST_NOT_FOUND;
		return item4;
	}

	if (want == WANT_A6_ONLY)
	{
		si_item_release(item4);
		if ((item6 == NULL) && (err != NULL)) *err = SI_STATUS_H_ERRNO_HOST_NOT_FOUND;
		return item6;
	}

	if ((item6 == NULL) && (item4 == NULL))
	{
		if (err != NULL) *err = SI_STATUS_H_ERRNO_HOST_NOT_FOUND;
		return NULL;
	}

	/* output item will have IPv6 + mapped IPv4 addresses */

	out = (build_hostent_t *)calloc(1, sizeof(build_hostent_t));
	if (out == NULL)
	{
		si_item_release(item4);
		si_item_release(item6);
		if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
		return NULL;
	}

	if (item4 != NULL)
	{
		h = (struct hostent *)((uintptr_t)item4 + sizeof(si_item_t));

		out->host.h_name = lower_case(h->h_name);

		if (h->h_aliases != NULL)
		{
			for (i = 0; h->h_aliases[i] != NULL; i++) merge_alias(h->h_aliases[i], out);
		}

		for (i = 0; h->h_addr_list[i] != 0; i++)
		{
			addr6.__u6_addr.__u6_addr32[0] = 0x00000000;
			addr6.__u6_addr.__u6_addr32[1] = 0x00000000;
			addr6.__u6_addr.__u6_addr32[2] = htonl(0x0000ffff);
			memmove(&(addr6.__u6_addr.__u6_addr32[3]), h->h_addr_list[i], IPV4_ADDR_LEN);
			append_addr((const char *)&addr6, IPV6_ADDR_LEN, out);
		}
	}

	if (item6 != NULL)
	{
		h = (struct hostent *)((uintptr_t)item6 + sizeof(si_item_t));

		if (out->host.h_name == NULL) out->host.h_name = lower_case(h->h_name);

		if (h->h_aliases != NULL)
		{
			for (i = 0; h->h_aliases[i] != NULL; i++) merge_alias(h->h_aliases[i], out);
		}

		for (i = 0; h->h_addr_list[i] != 0; i++) append_addr(h->h_addr_list[i], IPV6_ADDR_LEN, out);
	}

	si_item_release(item4);
	si_item_release(item6);

	unused = 0;

	item6 = (si_item_t *)LI_ils_create("L4488s*44c", (unsigned long)si, CATEGORY_HOST_IPV6, 1, unused, unused, out->host.h_name, out->host.h_aliases, AF_INET6, IPV6_ADDR_LEN, out->host.h_addr_list);

	free_build_hostent(out);

	return item6;
}
