/*
 * Copyright (c) 2004-2006, 2008, 2009, 2011-2013, 2015, 2017, 2018 Apple Inc. All rights reserved.
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

#ifndef __DNSINFO_CREATE_H__
#define __DNSINFO_CREATE_H__

/*
 * These routines provide access to the systems DNS configuration
 */

#include <TargetConditionals.h>
#include <os/availability.h>
#include <sys/cdefs.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <dnsinfo.h>

typedef const struct __dns_create_config *      dns_create_config_t;
typedef const struct __dns_create_resolver *    dns_create_resolver_t;

#ifndef _PATH_RESOLVER_DIR
#if	TARGET_OS_OSX
#define _PATH_RESOLVER_DIR "/etc/resolver"
#else
#define _PATH_RESOLVER_DIR "/Library/Preferences/SystemConfiguration/resolver"
#endif
#endif

__BEGIN_DECLS

/*
 * DNS configuration creation APIs
 */
dns_create_config_t
_dns_configuration_create       (void)						API_AVAILABLE(macos(10.4), ios(2.0));

void
_dns_configuration_add_resolver (dns_create_config_t	*_config,
				 dns_create_resolver_t	_resolver)		API_AVAILABLE(macos(10.4), ios(2.0));

void
_dns_configuration_signature	(dns_create_config_t	*_config,
				 unsigned char		*signature,
				 size_t			signature_len)		API_AVAILABLE(macos(10.7), ios(5.0));	// signature_len >= CC_SHA256_DIGEST_LENGTH

void
_dns_configuration_free		(dns_create_config_t	*_config)		API_AVAILABLE(macos(10.4), ios(2.0));

/*
 * DNS [resolver] configuration creation APIs
 */
dns_create_resolver_t
_dns_resolver_create		(void)						API_AVAILABLE(macos(10.4), ios(2.0));

void
_dns_resolver_set_domain	(dns_create_resolver_t	*_resolver,
				 const char		*domain)		API_AVAILABLE(macos(10.4), ios(2.0));

void
_dns_resolver_add_nameserver	(dns_create_resolver_t	*_resolver,
				 struct sockaddr	*nameserver)		API_AVAILABLE(macos(10.4), ios(2.0));

void
_dns_resolver_add_search	(dns_create_resolver_t	*_resolver,
				 const char		*search)		API_AVAILABLE(macos(10.4), ios(2.0));

void
_dns_resolver_add_sortaddr	(dns_create_resolver_t	*_resolver,
				 dns_sortaddr_t		*sortaddr)		API_AVAILABLE(macos(10.4), ios(2.0));

void
_dns_resolver_set_configuration_identifier
				(dns_create_resolver_t	*_resolver,
				 const char		*config_identifier)	API_AVAILABLE(macos(10.11), ios(9.0));

void
_dns_resolver_set_flags		(dns_create_resolver_t	*_resolver,
				 uint32_t		flags)			API_AVAILABLE(macos(10.7), ios(4.0));

void
_dns_resolver_set_if_index	(dns_create_resolver_t	*_resolver,
				 uint32_t		if_index,
				 const char		*if_name)		API_AVAILABLE(macos(10.7), ios(4.0));

void
_dns_resolver_set_options	(dns_create_resolver_t	*_resolver,
				 const char		*options)		API_AVAILABLE(macos(10.4), ios(2.0));

void
_dns_resolver_set_order		(dns_create_resolver_t	*_resolver,
				 uint32_t		order)			API_AVAILABLE(macos(10.4), ios(2.0));

void
_dns_resolver_set_port		(dns_create_resolver_t	*_resolver,
				 uint16_t		port)			API_AVAILABLE(macos(10.4), ios(2.0));	// host byte order

void
_dns_resolver_set_timeout	(dns_create_resolver_t	*_resolver,
				 uint32_t		timeout)		API_AVAILABLE(macos(10.4), ios(2.0));

void
_dns_resolver_set_service_identifier
				(dns_create_resolver_t	*_resolver,
				 uint32_t		service_identifier)	API_AVAILABLE(macos(10.9), ios(7.0));

void
_dns_resolver_free		(dns_create_resolver_t	*_resolver)		API_AVAILABLE(macos(10.4), ios(2.0));

/*
 * DNS [resolver] flat-file configuration creation APIs
 */
void
_dnsinfo_flatfile_add_resolvers	(dns_create_config_t	*config)		API_AVAILABLE(macos(10.6)) SPI_AVAILABLE(ios(10.15), tvos(13.0), watchos(6.0), bridgeos(6.0));

void
_dnsinfo_flatfile_set_flags	(uint32_t		flags)			API_AVAILABLE(macos(10.9)) SPI_AVAILABLE(ios(10.15), tvos(13.0), watchos(6.0), bridgeos(6.0));

__END_DECLS

#endif	/* __DNSINFO_CREATE_H__ */
