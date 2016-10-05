/*
 * Darling - DNS resolver module
 * Copyright (c) 2015 Lubos Dolezel, All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */

#include "../lookup.subproj/si_module.h"
#include <dispatch/dispatch.h>
#include <resolv.h>
#include <stdio.h>
#include <stdbool.h>

static int
dr_is_valid(si_mod_t *si, si_item_t *item)
{
	return 0;
}

static void
dr_close(si_mod_t *si)
{
}

static si_item_t *
dr_hostbyname(si_mod_t *si, const char *name, int af, const char *interface, uint32_t *err)
{
	int type, l, i, j, elem_length;
	unsigned char buf[4096];
	ns_msg msg;
	ns_rr rr;
	char* addr_buf;
	void** addr_ptrs;
	uint64_t ttl_end;

	if (af == AF_INET)
	{
		elem_length = 4;
		type = ns_t_a;
	}
	else if (af == AF_INET6)
	{
		elem_length = 16;
		type = ns_t_aaaa;
	}
	else
		return NULL;

	l = res_query(name, ns_c_in, type, buf, sizeof(buf));
	if (l < 0)
		return NULL;

	ns_initparse(buf, l, &msg);
	l = ns_msg_count(msg, ns_s_an);

	addr_buf = (char*) __builtin_alloca(l * elem_length);
	addr_ptrs = (void**) __builtin_alloca((l+1) * sizeof(void*));
	addr_ptrs[l] = NULL;

	for (i = 0, j = 0; i < l; i++)
	{
		const unsigned char* data;
		int type;

		ns_parserr(&msg, ns_s_an, i, &rr);
		data = ns_rr_rdata(rr);
		type = ns_rr_type(rr);

		if ((af == AF_INET && type == ns_t_a)
				|| (af == AF_INET6 && type == ns_t_aaaa))
		{
			memcpy(addr_buf + (j*elem_length), data, elem_length);
			addr_ptrs[j] = addr_buf + (j*elem_length);
			j++;
		}
	}
	addr_ptrs[j] = NULL;

	ttl_end = time(NULL) + rr.ttl;
	if (af == AF_INET)
	{
		return (si_item_t *)LI_ils_create("L4488s*44a", (unsigned long)si, CATEGORY_HOST_IPV4, 1, ttl_end,
				0LL, name, NULL, af, elem_length, addr_ptrs);
	}
	else if (af == AF_INET6)
	{
		return (si_item_t *)LI_ils_create("L4488s*44c", (unsigned long)si, CATEGORY_HOST_IPV6, 1, ttl_end,
				0LL, name, NULL, af, elem_length, addr_ptrs);
	}
	return NULL;
}

static si_item_t *
dr_hostbyaddr(si_mod_t *si, const void *addr, int af, const char *interface, uint32_t *err)
{
	char ptr[100];
	unsigned char buf[4096];
	int l, i;
	ns_msg msg;
	ns_rr rr;
	char** name_ptrs;

	if (af == AF_INET)
	{
		const unsigned char* ip = (const unsigned char*) addr;
		sprintf(ptr, "%d.%d.%d.%d.in-addr.arpa", ip[3], ip[2], ip[1], ip[0]);
	}
	else if (af == AF_INET6)
	{
		if (memcmp(addr, "\0\0\0\0\0\0\0\0\0\0\xff\xff", 12) == 0)
		{
			// This is a mapped IPv4 address
			const unsigned char* ip = ((const unsigned char*) addr) + 12;
			sprintf(ptr, "%d.%d.%d.%d.in-addr.arpa", ip[3], ip[2], ip[1], ip[0]);
		}
		else
		{
			const unsigned char* ip = (const unsigned char*) addr;
			const char* dig = "0123456789abcdef";
			int i;

			for (i = 0; i < 16; i++)
			{
				ptr[i*4] = dig[ip[i] & 0xf];
				ptr[i*4+1] = '.';
				ptr[i*4+2] = dig[ip[i] >> 4];
				ptr[i*4+3] = '.';
			}
			strcpy(&ptr[64], "ip6.arpa");
		}
	}
	else
		return NULL;

	l = res_query(ptr, ns_c_in, ns_t_ptr, buf, sizeof(buf));
	if (l < 0)
		return NULL;

	ns_initparse(buf, l, &msg);
	l = ns_msg_count(msg, ns_s_an);

	name_ptrs = (char**) __builtin_alloca((l+1) * sizeof(char*));
	name_ptrs[l] = NULL;

	for (i = 0; i < l; i++)
	{
		const unsigned char* data;
		int type;

		ns_parserr(&msg, ns_s_an, i, &rr);
		data = ns_rr_rdata(rr);
		type = ns_rr_type(rr);

		if (type == ns_t_ptr)
		{
			// TODO	
		}
	}

	return NULL;
}

static si_list_t *
dr_addrinfo(si_mod_t *si, const void *node, const void *serv, uint32_t family, uint32_t socktype, uint32_t proto, uint32_t flags, const char *interface, uint32_t *err)
{
	bool resolveV4, resolveV6;
	int servPort = 0;
	
	if (err != NULL) *err = SI_STATUS_NO_ERROR;
	
	switch (family)
	{
		case AF_INET6:
			resolveV4 = (flags & AI_V4MAPPED) != 0;
			resolveV6 = true;
			break;
		case AF_INET:
			resolveV4 = true;
			resolveV6 = false;
			break;
		case AF_UNSPEC:
			resolveV4 = true;
			resolveV6 = true;
			break;
		default:
			return NULL;
	}
	
	if (flags & AI_NUMERICSERV)
		servPort = *(uint16_t *)serv;
	else
	{
		if (_gai_serv_to_port(serv, proto, &servPort) != 0)
		{
			if (err)
				*err = SI_STATUS_EAI_NONAME;
			return NULL;
		}
	}
	
	if (flags & AI_NUMERICHOST)
	{
		char* cname = NULL;
		struct in_addr addr4;
		struct in6_addr addr6;
		
		if (family == AF_INET) {
			memcpy(&addr4, node, sizeof(addr4));
		} else if (family == AF_INET6) {
			memcpy(&addr6, node, sizeof(addr6));
		}
		
		return si_addrinfo_list(si, flags, socktype, proto,
				(family == AF_INET) ? &addr4 : NULL,
				(family == AF_INET6) ? &addr6 : NULL,
				servPort, 0, cname, cname);
	}
	else
	{
		int i, l;
		ns_msg msg;
		ns_rr rr;
		unsigned char buf[4096];
		si_list_t *out = NULL;
		si_list_t *list;
		
		if (resolveV6)
		{
			l = res_query((const char*) node, ns_c_in, ns_t_aaaa, buf, sizeof(buf));
			if (l < 0)
				goto after_v6;

			ns_initparse(buf, l, &msg);
			l = ns_msg_count(msg, ns_s_an);

			for (i = 0; i < l; i++)
			{
				const unsigned char* data;
				int type;

				ns_parserr(&msg, ns_s_an, i, &rr);
				data = ns_rr_rdata(rr);
				type = ns_rr_type(rr);

				if (type == ns_t_aaaa)
				{
					list = si_addrinfo_list(si, flags, socktype, proto, NULL,
							(struct in6_addr*) data, servPort, 0, NULL,
							(const char*) node);

					out = si_list_concat(out, list);
					si_list_release(list);
				}
			}
		}
		
		// If AI_V4MAPPED is used, return IPv4 mapped addresses only if there
		// are no IPv6 addresses or AI_ALL was also specified.
		if (resolveV4 && family == AF_INET6 && !(flags & AI_ALL))
		{
			if (out != NULL)
				resolveV4 = false;
		}
		
after_v6:
		if (resolveV4)
		{
			l = res_query((const char*) node, ns_c_in, ns_t_a, buf, sizeof(buf));
			if (l < 0)
				goto after_v4;

			ns_initparse(buf, l, &msg);
			l = ns_msg_count(msg, ns_s_an);
			
			for (i = 0; i < l; i++)
			{
				const unsigned char* data;
				int type;

				ns_parserr(&msg, ns_s_an, i, &rr);
				data = ns_rr_rdata(rr);
				type = ns_rr_type(rr);

				if (type == ns_t_a)
				{
					if (family == AF_INET6)
					{
						// Convert to a mapped address
						struct in6_addr mapped;

						memcpy(&mapped.__u6_addr, "\0\0\0\0\0\0\0\0\0\0\xff\xff", 12);
						mapped.__u6_addr.__u6_addr32[3] = *(uint32_t*) data;

						list = si_addrinfo_list(si, flags, socktype, proto, NULL,
							(struct in6_addr*) &mapped, servPort, 0, NULL,
							(const char*) node);

						out = si_list_concat(out, list);
						si_list_release(list);
					}
					else
					{
						list = si_addrinfo_list(si, flags, socktype, proto,
								(struct in_addr*) data, NULL, servPort, 0,
								(const char*) node, NULL);

						out = si_list_concat(out, list);
						si_list_release(list);
					}
				}
			}
		}
		
after_v4:
		
		return out;
	}
}

static si_item_t *
dr_item_call(si_mod_t *si, int call, const char *name, const char *ignored, const char *interface, uint32_t class, uint32_t type, uint32_t *err)
{
	return NULL;
}

static si_list_t *
dr_srv_byname(si_mod_t* si, const char *qname, const char *interface, uint32_t *err)
{
	return NULL;
}

si_mod_t *si_module_static_darling(void)
{
	static const struct si_mod_vtable_s dr_vtable =
	{
		.sim_close = &dr_close,
		.sim_is_valid = &dr_is_valid,
		.sim_host_byname = &dr_hostbyname,
		.sim_host_byaddr = &dr_hostbyaddr,
		.sim_item_call = &dr_item_call,
		.sim_addrinfo = &dr_addrinfo,
		.sim_srv_byname = &dr_srv_byname,
	};

	static si_mod_t si =
	{
		.vers = 1,
		.refcount = 1,
		.flags = SI_MOD_FLAG_STATIC,

		.private = NULL,
		.vtable = &dr_vtable,
	};

	static dispatch_once_t once;

	dispatch_once(&once, ^{
		si.name = strdup("darling-resolver");
	});

	return (si_mod_t*)&si;

}

