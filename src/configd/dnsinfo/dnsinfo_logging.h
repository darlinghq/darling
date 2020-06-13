/*
 * Copyright (c) 2017, 2018 Apple Inc. All rights reserved.
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

#ifndef _S_DNSINFO_LOGGING_H
#define _S_DNSINFO_LOGGING_H

#include <os/availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include "SCNetworkReachabilityLogging.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_var.h>

#include <dnsinfo.h>
#include "dnsinfo_private.h"

__BEGIN_DECLS

#ifndef	my_log
#define	my_log(__level, __format, ...)	SC_log(__level, __format, ## __VA_ARGS__)
#define	MY_LOG_DEFINED_LOCALLY
#endif	// !my_log

#ifndef	my_log_context_type
#define	MY_LOG_CONTEXT_TYPE_DEFINED_LOCALLY
#define	my_log_context_type	void *
#endif	// !my_log_context_type

#ifndef	my_log_context_name
#define	MY_LOG_CONTEXT_NAME_DEFINED_LOCALLY
#define	my_log_context_name	context
#endif	// !my_log_context_name

static __inline__ void
_dns_resolver_log(uint32_t version, dns_resolver_t *resolver, int index, Boolean debug, my_log_context_type my_log_context_name)
{
#if	defined(MY_LOG_CONTEXT_TYPE_DEFINED_LOCALLY) && defined(MY_LOG_CONTEXT_NAME_DEFINED_LOCALLY)
#pragma	unused(my_log_context_name)
#endif
	int			i;
	uint32_t		flags;
	char			reach_str[100];
	CFMutableStringRef	str;

	my_log(LOG_INFO, "%s", "");
	my_log(LOG_INFO, "resolver #%d", index);

	if (resolver->domain != NULL) {
		my_log(LOG_INFO, "  domain   : %s", resolver->domain);
	}

	for (i = 0; i < resolver->n_search; i++) {
		my_log(LOG_INFO, "  search domain[%d] : %s", i, resolver->search[i]);
	}

	for (i = 0; i < resolver->n_nameserver; i++) {
		char	buf[128];

		_SC_sockaddr_to_string(resolver->nameserver[i], buf, sizeof(buf));
		my_log(LOG_INFO, "  nameserver[%d] : %s", i, buf);
	}

	for (i = 0; i < resolver->n_sortaddr; i++) {
		char	abuf[32];
		char	mbuf[32];

		(void)inet_ntop(AF_INET, &resolver->sortaddr[i]->address, abuf, sizeof(abuf));
		(void)inet_ntop(AF_INET, &resolver->sortaddr[i]->mask,    mbuf, sizeof(mbuf));
		my_log(LOG_INFO, "  sortaddr[%d] : %s/%s", i, abuf, mbuf);
	}

	if (resolver->options != NULL) {
		my_log(LOG_INFO, "  options  : %s", resolver->options);
	}

	if (resolver->port != 0) {
		my_log(LOG_INFO, "  port     : %hd", resolver->port);
	}

	if (resolver->timeout != 0) {
		my_log(LOG_INFO, "  timeout  : %d", resolver->timeout);
	}

	if (resolver->if_index != 0) {
#ifndef	_LIBLOG_SYSTEMCONFIGURATION_
		char	buf[IFNAMSIZ];
#endif	// !_LIBLOG_SYSTEMCONFIGURATION_
		char	*if_name	= NULL;

		if ((version >= 20170629) && (resolver->if_name != NULL)) {
			if_name = resolver->if_name;
#ifndef	_LIBLOG_SYSTEMCONFIGURATION_
		} else {
			if_name = if_indextoname(resolver->if_index, buf);
#endif	// !_LIBLOG_SYSTEMCONFIGURATION_
		}
		my_log(LOG_INFO, "  if_index : %d (%s)",
		       resolver->if_index,
		       (if_name != NULL) ? if_name : "?");
	}

	if (resolver->service_identifier != 0) {
		my_log(LOG_INFO, "  service_identifier : %d",
		       resolver->service_identifier);
	}

	flags = resolver->flags;
	str = CFStringCreateMutable(NULL, 0);
	CFStringAppend(str, CFSTR("  flags    : "));
	if (debug) {
		CFStringAppendFormat(str, NULL, CFSTR("0x%08x"), flags);
	}
	if (flags != 0) {
		if (debug) {
			CFStringAppendFormat(str, NULL, CFSTR(" ("));
		}
		if (flags & DNS_RESOLVER_FLAGS_SCOPED) {
			flags &= ~DNS_RESOLVER_FLAGS_SCOPED;
			CFStringAppendFormat(str, NULL, CFSTR("Scoped%s"), flags != 0 ? ", " : "");
		}
		if (flags & DNS_RESOLVER_FLAGS_SERVICE_SPECIFIC) {
			flags &= ~DNS_RESOLVER_FLAGS_SERVICE_SPECIFIC;
			CFStringAppendFormat(str, NULL, CFSTR("Service-specific%s"), flags != 0 ? ", " : "");
		}
		if (flags & DNS_RESOLVER_FLAGS_SUPPLEMENTAL) {
			flags &= ~DNS_RESOLVER_FLAGS_SUPPLEMENTAL;
			CFStringAppendFormat(str, NULL, CFSTR("Supplemental%s"), flags != 0 ? ", " : "");
		}
		if (flags & DNS_RESOLVER_FLAGS_REQUEST_A_RECORDS) {
			flags &= ~DNS_RESOLVER_FLAGS_REQUEST_A_RECORDS;
			CFStringAppendFormat(str, NULL, CFSTR("Request A records%s"), flags != 0 ? ", " : "");
		}
		if (flags & DNS_RESOLVER_FLAGS_REQUEST_AAAA_RECORDS) {
			flags &= ~DNS_RESOLVER_FLAGS_REQUEST_AAAA_RECORDS;
			CFStringAppendFormat(str, NULL, CFSTR("Request AAAA records%s"), flags != 0 ? ", " : "");
		}
		if (flags != 0) {
			CFStringAppendFormat(str, NULL, CFSTR("0x%08x"), flags);
		}
		if (debug) {
			CFStringAppend(str, CFSTR(")"));
		}
	}
	my_log(LOG_INFO, "%@", str);
	CFRelease(str);

	__SCNetworkReachability_flags_string(resolver->reach_flags, TRUE, reach_str, sizeof(reach_str));
	my_log(LOG_INFO, "  reach    : %s", reach_str);

	if (resolver->search_order != 0) {
		my_log(LOG_INFO, "  order    : %d", resolver->search_order);
	}

	if (debug && (resolver->cid != NULL)) {
		my_log(LOG_INFO, "  config id: %s", resolver->cid);
	}

	return;
}

static __inline__ void
_dns_configuration_log(dns_config_t *dns_config, Boolean debug, my_log_context_type my_log_context_name)
{
#if	defined(MY_LOG_CONTEXT_TYPE_DEFINED_LOCALLY) && defined(MY_LOG_CONTEXT_NAME_DEFINED_LOCALLY)
#pragma	unused(my_log_context_name)
#endif
	int	i;

	my_log(LOG_INFO, "%s", "DNS configuration");

	for (i = 0; i < dns_config->n_resolver; i++) {
		dns_resolver_t	*resolver	= dns_config->resolver[i];

		_dns_resolver_log(dns_config->version, resolver, i + 1, debug, my_log_context_name);
	}

	if ((dns_config->n_scoped_resolver > 0) && (dns_config->scoped_resolver != NULL)) {
		my_log(LOG_INFO, "%s", "");
		my_log(LOG_INFO, "%s", "DNS configuration (for scoped queries)");

		for (i = 0; i < dns_config->n_scoped_resolver; i++) {
			dns_resolver_t	*resolver	= dns_config->scoped_resolver[i];

			_dns_resolver_log(dns_config->version, resolver, i + 1, debug, my_log_context_name);
		}
	}

	if ((dns_config->n_service_specific_resolver > 0) && (dns_config->service_specific_resolver != NULL)) {
		my_log(LOG_INFO, "%s", "");
		my_log(LOG_INFO, "%s", "DNS configuration (for service-specific queries)");

		for (i = 0; i < dns_config->n_service_specific_resolver; i++) {
			dns_resolver_t	*resolver	= dns_config->service_specific_resolver[i];

			_dns_resolver_log(dns_config->version, resolver, i + 1, debug, my_log_context_name);
		}
	}

	return;
}

#ifdef	MY_LOG_DEFINED_LOCALLY
#undef	my_log
#undef	MY_LOG_DEFINED_LOCALLY
#endif	// MY_LOG_DEFINED_LOCALLY

#ifdef	MY_LOG_CONTEXT_TYPE_DEFINED_LOCALLY
#undef	my_log_context_type
#undef	MY_LOG_CONTEXT_TYPE_DEFINED_LOCALLY
#endif	// MY_LOG_CONTEXT_TYPE_DEFINED_LOCALLY

#ifdef	MY_LOG_CONTEXT_NAME_DEFINED_LOCALLY
#undef	my_log_context_name
#undef	MY_LOG_CONTEXT_NAME_DEFINED_LOCALLY
#endif	// MY_LOG_CONTEXT_NAME_DEFINED_LOCALLY

__END_DECLS

#endif	/* !_S_DNSINFO_LOGGING_H */
