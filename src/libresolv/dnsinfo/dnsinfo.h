/*
 * Copyright (c) 2004-2006, 2008, 2009, 2011-2013 Apple Inc. All rights reserved.
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

#ifndef __DNSINFO_H__
#define __DNSINFO_H__

/*
 * These routines provide access to the systems DNS configuration
 */

#include <Availability.h>
#include <sys/cdefs.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define	DNSINFO_VERSION		20130402

#define DEFAULT_SEARCH_ORDER    200000   /* search order for the "default" resolver domain name */

#define	DNS_PTR(type, name)				\
	union {						\
		type		name;			\
		uint64_t	_ ## name ## _p;	\
	}

#define	DNS_VAR(type, name)				\
	type	name


#pragma pack(4)
typedef struct {
	struct in_addr	address;
	struct in_addr	mask;
} dns_sortaddr_t;
#pragma pack()


#pragma pack(4)
typedef struct {
	DNS_PTR(char *,			domain);	/* domain */
	DNS_VAR(int32_t,		n_nameserver);	/* # nameserver */
	DNS_PTR(struct sockaddr **,	nameserver);
	DNS_VAR(uint16_t,		port);		/* port (in host byte order) */
	DNS_VAR(int32_t,		n_search);	/* # search */
	DNS_PTR(char **,		search);
	DNS_VAR(int32_t,		n_sortaddr);	/* # sortaddr */
	DNS_PTR(dns_sortaddr_t **,	sortaddr);
	DNS_PTR(char *,			options);	/* options */
	DNS_VAR(uint32_t,		timeout);	/* timeout */
	DNS_VAR(uint32_t,		search_order);	/* search_order */
	DNS_VAR(uint32_t,		if_index);
	DNS_VAR(uint32_t,		flags);
	DNS_VAR(uint32_t,		reach_flags);	/* SCNetworkReachabilityFlags */
	DNS_VAR(uint32_t,		service_identifier);
	DNS_VAR(uint32_t,		reserved[4]);
} dns_resolver_t;
#pragma pack()


#define DNS_RESOLVER_FLAGS_SCOPED		1		/* configuration is for scoped questions */
#define DNS_RESOLVER_FLAGS_REQUEST_A_RECORDS	2		/* always requesting for A dns records in queries */
#define DNS_RESOLVER_FLAGS_REQUEST_AAAA_RECORDS	4		/* always requesting for AAAA dns records in queries */
#define DNS_RESOLVER_FLAGS_SERVICE_SPECIFIC	8		/* configuration is service-specific */

#define	DNS_RESOLVER_FLAGS_REQUEST_ALL_RECORDS	\
	(DNS_RESOLVER_FLAGS_REQUEST_A_RECORDS | DNS_RESOLVER_FLAGS_REQUEST_AAAA_RECORDS)

#pragma pack(4)
typedef struct {
	DNS_VAR(int32_t,		n_resolver);		/* resolver configurations */
	DNS_PTR(dns_resolver_t **,	resolver);
	DNS_VAR(int32_t,		n_scoped_resolver);	/* "scoped" resolver configurations */
	DNS_PTR(dns_resolver_t **,	scoped_resolver);
	DNS_VAR(uint64_t,		generation);
	DNS_VAR(int32_t,		n_service_specific_resolver);
	DNS_PTR(dns_resolver_t **,	service_specific_resolver);
} dns_config_t;
#pragma pack()


__BEGIN_DECLS

/*
 * DNS configuration access APIs
 */
const char *
dns_configuration_notify_key    ()				__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0);

dns_config_t *
dns_configuration_copy		()				__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0);

void
dns_configuration_free		(dns_config_t	*config)	__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0);

void
_dns_configuration_ack		(dns_config_t	*config,
				 const char	*bundle_id)	__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

__END_DECLS

#endif	/* __DNSINFO_H__ */
