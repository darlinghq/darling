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

#ifndef __DNSINFO_CREATE_H__
#define __DNSINFO_CREATE_H__

/*
 * These routines provide access to the systems DNS configuration
 */

#include <TargetConditionals.h>
#include <Availability.h>
#include <sys/cdefs.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <dnsinfo.h>

typedef const struct __dns_create_config *      dns_create_config_t;
typedef const struct __dns_create_resolver *    dns_create_resolver_t;

#if	!TARGET_OS_IPHONE
#ifndef _PATH_RESOLVER_DIR
#define _PATH_RESOLVER_DIR "/etc/resolver"
#endif
#endif	/* !TARGET_OS_IPHONE */

__BEGIN_DECLS

/*
 * DNS configuration creation APIs
 */
dns_create_config_t
_dns_configuration_create       (void)						__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0);

void
_dns_configuration_add_resolver (dns_create_config_t	*_config,
				 dns_create_resolver_t	_resolver)		__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0);

void
_dns_configuration_signature	(dns_create_config_t	*_config,
				 unsigned char		*signature,
				 size_t			signature_len)		__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_5_0);	// signature_len >= CC_SHA1_DIGEST_LENGTH

void
_dns_configuration_free		(dns_create_config_t	*_config)		__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0);

/*
 * DNS [resolver] configuration creation APIs
 */
dns_create_resolver_t
_dns_resolver_create		(void)						__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0);

void
_dns_resolver_set_domain	(dns_create_resolver_t	*_resolver,
				 const char		*domain)		__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0);

void
_dns_resolver_add_nameserver	(dns_create_resolver_t	*_resolver,
				 struct sockaddr	*nameserver)		__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0);

void
_dns_resolver_add_search	(dns_create_resolver_t	*_resolver,
				 const char		*search)		__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0);

void
_dns_resolver_add_sortaddr	(dns_create_resolver_t	*_resolver,
				 dns_sortaddr_t		*sortaddr)		__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0);

void
_dns_resolver_set_flags		(dns_create_resolver_t	*_resolver,
				 uint32_t		flags)			__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0);

void
_dns_resolver_set_if_index	(dns_create_resolver_t	*_resolver,
				 uint32_t		if_index)		__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0);

void
_dns_resolver_set_options	(dns_create_resolver_t	*_resolver,
				 const char		*options)		__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0);

void
_dns_resolver_set_order		(dns_create_resolver_t	*_resolver,
				 uint32_t		order)			__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0);

void
_dns_resolver_set_port		(dns_create_resolver_t	*_resolver,
				 uint16_t		port)			__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0);	// host byte order

void
_dns_resolver_set_timeout	(dns_create_resolver_t	*_resolver,
				 uint32_t		timeout)		__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0);

void
_dns_resolver_set_service_identifier	(dns_create_resolver_t	*_resolver,
					uint32_t		service_identifier)	__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

void
_dns_resolver_free		(dns_create_resolver_t	*_resolver)		__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0);

#if	!TARGET_OS_IPHONE
/*
 * DNS [resolver] flat-file configuration creation APIs
 */
void
_dnsinfo_flatfile_add_resolvers	(dns_create_config_t	*config)		__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_NA);

void
_dnsinfo_flatfile_set_flags	(uint32_t		flags)			__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_NA);
#endif	// !TARGET_OS_IPHONE

__END_DECLS

#endif	/* __DNSINFO_CREATE_H__ */
