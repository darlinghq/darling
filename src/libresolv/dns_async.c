/*
 * Copyright (c) 2003 - 2008 Apple, Inc. All rights reserved.
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <mach/mach.h>
#include <pthread.h>
#include <netdb.h>
#include <netdb_async.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dns.h>
#include <dns_util.h>
#include <stdarg.h>
#include <si_module.h>

extern void *LI_ils_create(char *fmt, ...);

/*
 typedef void (*dns_async_callback)(int32_t status, char *buf, uint32_t len, struct sockaddr *from, int fromlen, void *context);
*/

typedef struct
{
	void *orig_callback;
	void *orig_context;
	si_mod_t *dns;
} _dns_context_t;

static void
_dns_callback(si_item_t *item, uint32_t status, void *ctx)
{
	_dns_context_t *my_ctx;
	si_dnspacket_t *res;
	char *packet;
	struct sockaddr *from;
	uint32_t pl;
	int fl;

	if (ctx == NULL) return;
	
	my_ctx = (_dns_context_t *)ctx;
	if (my_ctx->orig_callback == NULL)
	{
		si_item_release(item);
		si_module_release(my_ctx->dns);
		free(my_ctx);
		return;
	}

	if (status >= SI_STATUS_INTERNAL) status = NO_RECOVERY;

	packet = NULL;
	pl = 0;
	from = NULL;
	fl = 0;

	res = NULL;
	if (item != NULL) res = (si_dnspacket_t *)((char *)item + sizeof(si_item_t));

	if ((res != NULL) && (res->dns_packet_len > 0))
	{
		packet = malloc(res->dns_packet_len);
		if (packet == NULL) status = NO_RECOVERY;
		else
		{
			pl = res->dns_packet_len;
			memcpy(packet, res->dns_packet, res->dns_packet_len);

			if (res->dns_server_len > 0)
			{
				from = malloc(res->dns_server_len);
				if (from == NULL)
				{
					status = NO_RECOVERY;
					free(packet);
					packet = NULL;
					pl = 0;
				}
				else
				{
					fl = res->dns_server_len;
					memcpy(from, res->dns_server, res->dns_server_len);
				}
			}
		}
	}

	si_item_release(item);

	((dns_async_callback)(my_ctx->orig_callback))(status, packet, pl, from, fl, my_ctx->orig_context);

	si_module_release(my_ctx->dns);
	free(my_ctx);
}

int32_t
dns_async_start(mach_port_t *p, const char *name, uint16_t dnsclass, uint16_t dnstype, uint32_t do_search, dns_async_callback callback, void *context)
{
	si_mod_t *dns;
	int call;
	uint32_t c, t;
	_dns_context_t *my_ctx;

	*p = MACH_PORT_NULL;

	if (name == NULL) return NO_RECOVERY;

	dns = si_module_with_name("mdns");
	if (dns == NULL) return NO_RECOVERY;

	my_ctx = (_dns_context_t *)calloc(1, sizeof(_dns_context_t));
	if (my_ctx == NULL)
	{
		si_module_release(dns);
		return NO_RECOVERY;
	}

	my_ctx->orig_callback = callback;
	my_ctx->orig_context = context;
	my_ctx->dns = dns;

	call = SI_CALL_DNS_QUERY;
	if (do_search != 0) call = SI_CALL_DNS_SEARCH;

	c = dnsclass;
	t = dnstype;

	*p = si_async_call(dns, call, name, NULL, NULL, c, t, 0, 0, (void *)_dns_callback, (void *)my_ctx);
	if (*p == MACH_PORT_NULL)
	{
		free(my_ctx);
		si_module_release(dns);
		return NO_RECOVERY;
	}

	return 0;
}

void
dns_async_cancel(mach_port_t p)
{
	si_async_cancel(p);
}

int32_t
dns_async_send(mach_port_t *p, const char *name, uint16_t dnsclass, uint16_t dnstype, uint32_t do_search)
{
	return dns_async_start(p, name, dnsclass, dnstype, do_search, NULL, NULL);
}

/* unsupported */
int32_t
dns_async_receive(mach_port_t p, char **buf, uint32_t *len, struct sockaddr **from, uint32_t *fromlen)
{
	return 0;
}

int32_t
dns_async_handle_reply(void *msg)
{
	si_async_handle_reply(msg);
	return 0;
}
