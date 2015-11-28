/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

#ifndef _S_DNSINFO_INTERNAL_H
#define _S_DNSINFO_INTERNAL_H

#include <Availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include "SCNetworkReachabilityInternal.h"
#include <arpa/inet.h>

#include <dnsinfo.h>

__BEGIN_DECLS

static __inline__ void
_dns_resolver_print(dns_resolver_t *resolver, int index)
{
	int	i;

	SCPrint(TRUE, stdout, CFSTR("\nresolver #%d\n"), index);

	if (resolver->domain != NULL) {
		SCPrint(TRUE, stdout, CFSTR("  domain   : %s\n"), resolver->domain);
	}

	for (i = 0; i < resolver->n_search; i++) {
		SCPrint(TRUE, stdout, CFSTR("  search domain[%d] : %s\n"), i, resolver->search[i]);
	}

	for (i = 0; i < resolver->n_nameserver; i++) {
		char	buf[128];

		_SC_sockaddr_to_string(resolver->nameserver[i], buf, sizeof(buf));
		SCPrint(TRUE, stdout, CFSTR("  nameserver[%d] : %s\n"), i, buf);
	}

	for (i = 0; i < resolver->n_sortaddr; i++) {
		char	abuf[32];
		char	mbuf[32];

		(void)inet_ntop(AF_INET, &resolver->sortaddr[i]->address, abuf, sizeof(abuf));
		(void)inet_ntop(AF_INET, &resolver->sortaddr[i]->mask,    mbuf, sizeof(mbuf));
		SCPrint(TRUE, stdout, CFSTR("  sortaddr[%d] : %s/%s\n"), i, abuf, mbuf);
	}

	if (resolver->options != NULL) {
		SCPrint(TRUE, stdout, CFSTR("  options  : %s\n"), resolver->options);
	}

	if (resolver->port != 0) {
		SCPrint(TRUE, stdout, CFSTR("  port     : %hd\n"), resolver->port);
	}

	if (resolver->timeout != 0) {
		SCPrint(TRUE, stdout, CFSTR("  timeout  : %d\n"), resolver->timeout);
	}

	if (resolver->if_index != 0) {
		char	buf[IFNAMSIZ];
		char	*if_name;

		if_name = if_indextoname(resolver->if_index, buf);
		SCPrint(TRUE, stdout, CFSTR("  if_index : %d (%s)\n"),
			resolver->if_index,
			(if_name != NULL) ? if_name : "?");
	}

	if (resolver->service_identifier != 0) {
		SCPrint(TRUE, stdout, CFSTR("  service_identifier : %d\n"),
			resolver->service_identifier);
	}

	if (resolver->flags != 0) {
		uint32_t	flags	= resolver->flags;

		SCPrint(TRUE, stdout, CFSTR("  flags    : "));
		SCPrint(_sc_debug, stdout, CFSTR("0x%08x ("), flags);
		if (flags & DNS_RESOLVER_FLAGS_SCOPED) {
			SCPrint(TRUE, stdout, CFSTR("Scoped"));
			flags &= ~DNS_RESOLVER_FLAGS_SCOPED;
			SCPrint(flags != 0, stdout, CFSTR(", "));
		}
		if (flags & DNS_RESOLVER_FLAGS_SERVICE_SPECIFIC) {
			SCPrint(TRUE, stdout, CFSTR("Service-specific"));
			flags &= ~DNS_RESOLVER_FLAGS_SERVICE_SPECIFIC;
			SCPrint(flags != 0, stdout, CFSTR(", "));
		}
		if (flags & DNS_RESOLVER_FLAGS_REQUEST_A_RECORDS) {
			SCPrint(TRUE, stdout, CFSTR("Request A records"));
			flags &= ~DNS_RESOLVER_FLAGS_REQUEST_A_RECORDS;
			SCPrint(flags != 0, stdout, CFSTR(", "));
		}
		if (flags & DNS_RESOLVER_FLAGS_REQUEST_AAAA_RECORDS) {
			SCPrint(TRUE, stdout, CFSTR("Request AAAA records"));
			flags &= ~DNS_RESOLVER_FLAGS_REQUEST_AAAA_RECORDS;
			SCPrint(flags != 0, stdout, CFSTR(", "));
		}
		if (flags != 0) {
			SCPrint(TRUE, stdout, CFSTR("0x%08x"), flags);
		}
		SCPrint(_sc_debug, stdout, CFSTR(")"));
		SCPrint(TRUE, stdout, CFSTR("\n"));
	}

	if (resolver->reach_flags != 0) {
		uint32_t	flags	= resolver->reach_flags;

		SCPrint(TRUE, stdout, CFSTR("  reach    : "));
		SCPrint(_sc_debug, stdout, CFSTR("0x%08x ("), flags);
		__SCNetworkReachabilityPrintFlags(flags);
		SCPrint(_sc_debug, stdout, CFSTR(")"));
		SCPrint(TRUE, stdout, CFSTR("\n"));
	}

	if (resolver->search_order != 0) {
		SCPrint(TRUE, stdout, CFSTR("  order    : %d\n"), resolver->search_order);
	}

	return;
}


static __inline__ void
_dns_configuration_print(dns_config_t *dns_config)
{
	int	i;

	SCPrint(TRUE, stdout, CFSTR("DNS configuration\n"));

	for (i = 0; i < dns_config->n_resolver; i++) {
		dns_resolver_t	*resolver	= dns_config->resolver[i];

		_dns_resolver_print(resolver, i + 1);
	}

	if ((dns_config->n_scoped_resolver > 0) && (dns_config->scoped_resolver != NULL)) {
		SCPrint(TRUE, stdout, CFSTR("\nDNS configuration (for scoped queries)\n"));

		for (i = 0; i < dns_config->n_scoped_resolver; i++) {
			dns_resolver_t	*resolver	= dns_config->scoped_resolver[i];

			_dns_resolver_print(resolver, i + 1);
		}
	}

	if ((dns_config->n_service_specific_resolver > 0) && (dns_config->service_specific_resolver != NULL)) {
		SCPrint(TRUE, stdout, CFSTR("\nDNS configuration (for service-specific queries)\n"));

		for (i = 0; i < dns_config->n_service_specific_resolver; i++) {
			dns_resolver_t	*resolver	= dns_config->service_specific_resolver[i];

			_dns_resolver_print(resolver, i + 1);
		}
	}

	return;
}

__END_DECLS

#endif	/* !_S_DNSINFO_INTERNAL_H */
