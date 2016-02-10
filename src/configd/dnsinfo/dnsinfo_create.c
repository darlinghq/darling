/*
 * Copyright (c) 2004-2006, 2009, 2011-2013 Apple Inc. All rights reserved.
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
#include <strings.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/mach_time.h>
#include <CommonCrypto/CommonDigest.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>

#include "dnsinfo_create.h"
#include "dnsinfo_private.h"
#include "network_information_priv.h"

#include "ip_plugin.h"


#define ROUNDUP(a, size) \
	(((a) & ((size)-1)) ? (1 + ((a) | ((size)-1))) : (a))


/*
 * to avoid extra calls to realloc() we want to pre-allocate the initial
 * resolver and configuration buffers of a sufficient size that they would
 * not normally need to be expanded.
 */
#define INITIAL_CONFIGURATION_BUF_SIZE  8192
#define INITIAL_RESOLVER_BUF_SIZE       1024


/*
 * DNS [configuration] buffer functions
 */


__private_extern__
dns_create_config_t
_dns_configuration_create()
{
	_dns_config_buf_t	*config;

	config = calloc(1, INITIAL_CONFIGURATION_BUF_SIZE);
	config->config.generation = mach_absolute_time();
//	config->n_attribute = 0;
//	config->n_padding = 0;
	return (dns_create_config_t)config;
}


static void
config_add_attribute(dns_create_config_t	*_config,
		     uint32_t			attribute_type,
		     uint32_t			attribute_length,
		     void			*attribute,
		     uint32_t			extra_padding)
{
	_dns_config_buf_t	*config	= (_dns_config_buf_t *)*_config;
	dns_attribute_t		*header;
	int			i;
	uint32_t		newLen;
	uint32_t		newSize;
	uint32_t		oldLen;
	uint32_t		rounded_length;

	// add space

	oldLen         = ntohl(config->n_attribute);
	rounded_length = ROUNDUP(attribute_length, sizeof(uint32_t));
	newLen         = sizeof(dns_attribute_t) + rounded_length;
	newSize = sizeof(_dns_config_buf_t) + oldLen + newLen;
	if (newSize > INITIAL_CONFIGURATION_BUF_SIZE) {
		config = realloc(config, newSize);
	}
	config->n_attribute = htonl(ntohl(config->n_attribute) + newLen);

	// increment additional padding that will be needed (later)
	config->n_padding = htonl(ntohl(config->n_padding) + extra_padding);

	// add attribute [header]

	/* ALIGN: _dns_config_buf_t is int aligned */
	header = (dns_attribute_t *)(void *)&config->attribute[oldLen];
	header->type   = htonl(attribute_type);
	header->length = htonl(newLen);

	// add attribute [data]

	bcopy(attribute, &header->attribute[0], attribute_length);
	for (i = attribute_length; i < rounded_length; i++) {
		header->attribute[i] = 0;
	}

	*_config = (dns_create_config_t)config;
	return;
}


static void
_dns_resolver_set_reach_flags(dns_create_resolver_t _resolver);


__private_extern__
void
_dns_configuration_add_resolver(dns_create_config_t     *_config,
				dns_create_resolver_t	_resolver)
{
	_dns_config_buf_t	*config		= (_dns_config_buf_t *)*_config;
	uint32_t		padding		= 0;
	_dns_resolver_buf_t	*resolver	= (_dns_resolver_buf_t *)_resolver;

	/*
	 * add reachability flags for this resovler configuration
	 */
	_dns_resolver_set_reach_flags(_resolver);

	/*
	 * compute the amount of space that will be needed for
	 * pointers to the resolver, the nameservers, the search
	 * list, and the sortaddr list.
	 */
	padding += sizeof(DNS_PTR(dns_resolver_t *, x));
	if (resolver->resolver.n_nameserver != 0) {
		padding += ntohl(resolver->resolver.n_nameserver) * sizeof(DNS_PTR(struct sockaddr *, x));
	}
	if (resolver->resolver.n_search != 0) {
		padding += ntohl(resolver->resolver.n_search) * sizeof(DNS_PTR(char *, x));
	}
	if (resolver->resolver.n_sortaddr != 0) {
		padding += ntohl(resolver->resolver.n_sortaddr) * sizeof(DNS_PTR(dns_sortaddr_t *, x));
	}

	if ((ntohl(resolver->resolver.flags) & DNS_RESOLVER_FLAGS_SCOPED)) {
		config->config.n_scoped_resolver = htonl(ntohl(config->config.n_scoped_resolver) + 1);
		config_add_attribute(_config,
				     CONFIG_ATTRIBUTE_SCOPED_RESOLVER,
				     sizeof(_dns_resolver_buf_t) + ntohl(resolver->n_attribute),
				     (void *)resolver,
				     padding);
	} else if ((ntohl(resolver->resolver.flags) & DNS_RESOLVER_FLAGS_SERVICE_SPECIFIC)) {
		config->config.n_service_specific_resolver = htonl(ntohl(config->config.n_service_specific_resolver) + 1);
		config_add_attribute(_config,
				     CONFIG_ATTRIBUTE_SERVICE_SPECIFIC_RESOLVER,
				     sizeof(_dns_resolver_buf_t) + ntohl(resolver->n_attribute),
				     (void *)resolver,
				     padding);
	} else {
		config->config.n_resolver = htonl(ntohl(config->config.n_resolver) + 1);
		config_add_attribute(_config,
				     CONFIG_ATTRIBUTE_RESOLVER,
				     sizeof(_dns_resolver_buf_t) + ntohl(resolver->n_attribute),
				     (void *)resolver,
				     padding);
	}

	return;
}


__private_extern__
void
_dns_configuration_signature(dns_create_config_t	*_config,
			     unsigned char		*signature,
			     size_t			signature_len)
{
	bzero(signature, signature_len);

	if (_config != NULL) {
		_dns_config_buf_t	*config	= (_dns_config_buf_t *)*_config;

		if (config != NULL) {
			CC_SHA1_CTX	ctx;
			uint64_t	generation_save;
			unsigned char	*sha1;
			unsigned char	sha1_buf[CC_SHA1_DIGEST_LENGTH];

			generation_save = config->config.generation;
			config->config.generation = 0;

			sha1 = (signature_len >= CC_SHA1_DIGEST_LENGTH) ? signature : sha1_buf;
			CC_SHA1_Init(&ctx);
			CC_SHA1_Update(&ctx,
				       config,
				       sizeof(_dns_config_buf_t) + ntohl(config->n_attribute));
			CC_SHA1_Final(sha1, &ctx);
			if (sha1 != signature) {
				bcopy(sha1, signature, signature_len);
			}

			config->config.generation = generation_save;
		}
	}

	return;
}


__private_extern__
void
_dns_configuration_free(dns_create_config_t *_config)
{
	_dns_config_buf_t	*config	= (_dns_config_buf_t *)*_config;

	free(config);
	*_config = NULL;
	return;
}


/*
 * DNS resolver configuration functions
 */

__private_extern__
dns_create_resolver_t
_dns_resolver_create()
{
	_dns_resolver_buf_t	*buf;

	buf = calloc(1, INITIAL_RESOLVER_BUF_SIZE);
//	buf->n_attribute = 0;
	return (dns_create_resolver_t)buf;
}


static void
_dns_resolver_add_attribute(dns_create_resolver_t	*_resolver,
			    uint32_t			attribute_type,
			    uint32_t			attribute_length,
			    void			*attribute)
{
	dns_attribute_t		*header;
	int			i;
	uint32_t		newLen;
	uint32_t		newSize;
	uint32_t		oldLen;
	_dns_resolver_buf_t	*resolver	= (_dns_resolver_buf_t *)*_resolver;
	uint32_t		rounded_length;

	// add space

	oldLen         = ntohl(resolver->n_attribute);
	rounded_length = ROUNDUP(attribute_length, sizeof(uint32_t));
	newLen         = sizeof(dns_attribute_t) + rounded_length;
	newSize = sizeof(_dns_resolver_buf_t) + oldLen + newLen;
	if (newSize > INITIAL_RESOLVER_BUF_SIZE) {
		resolver = realloc(resolver, newSize);
	}
	resolver->n_attribute = htonl(ntohl(resolver->n_attribute) + newLen);

	// add attribute [header]

	/* ALIGN: _dns_resolver_buf_t is int aligned */
	header = (dns_attribute_t *)(void *)&resolver->attribute[oldLen];
	header->type   = htonl(attribute_type);
	header->length = htonl(newLen);

	// add attribute [data]

	bcopy(attribute, &header->attribute[0], attribute_length);
	for (i = attribute_length; i < rounded_length; i++) {
		header->attribute[i] = 0;
	}

	*_resolver = (dns_create_resolver_t)resolver;
	return;
}


__private_extern__
void
_dns_resolver_add_nameserver(dns_create_resolver_t *_resolver, struct sockaddr *nameserver)
{
	uint32_t		new_flags	= 0;
	uint32_t		old_flags;
	_dns_resolver_buf_t	*resolver	= (_dns_resolver_buf_t *)*_resolver;

	old_flags = ntohl(resolver->resolver.flags);

	switch (nameserver->sa_family) {
		case AF_INET:
			if (ntohl(((struct sockaddr_in*)(void *)nameserver)->sin_addr.s_addr) == INADDR_LOOPBACK) {
				new_flags = DNS_RESOLVER_FLAGS_REQUEST_ALL_RECORDS;
			}
			break;
		case AF_INET6:
			if (IN6_IS_ADDR_LOOPBACK(&((struct sockaddr_in6 *)(void *)nameserver)->sin6_addr)){
				new_flags = DNS_RESOLVER_FLAGS_REQUEST_ALL_RECORDS;
			}
			break;
		default:
			break;
	}

	resolver->resolver.n_nameserver = htonl(ntohl(resolver->resolver.n_nameserver) + 1);
	_dns_resolver_add_attribute(_resolver, RESOLVER_ATTRIBUTE_ADDRESS, nameserver->sa_len, (void *)nameserver);

	if (new_flags != 0) {
		// if DNS request flags not explicitly set and we are
		// adding a LOOPBACK resolver address
		_dns_resolver_set_flags(_resolver, old_flags | new_flags);
	}
	return;
}


__private_extern__
void
_dns_resolver_add_search(dns_create_resolver_t *_resolver, const char *search)
{
	_dns_resolver_buf_t	*resolver	= (_dns_resolver_buf_t *)*_resolver;

	resolver->resolver.n_search = htonl(ntohl(resolver->resolver.n_search) + 1);
	_dns_resolver_add_attribute(_resolver, RESOLVER_ATTRIBUTE_SEARCH, (uint32_t)strlen(search) + 1, (void *)search);
	return;
}


__private_extern__
void
_dns_resolver_add_sortaddr(dns_create_resolver_t *_resolver, dns_sortaddr_t *sortaddr)
{
	_dns_resolver_buf_t	*resolver	= (_dns_resolver_buf_t *)*_resolver;

	resolver->resolver.n_sortaddr = htonl(ntohl(resolver->resolver.n_sortaddr) + 1);
	_dns_resolver_add_attribute(_resolver, RESOLVER_ATTRIBUTE_SORTADDR, (uint32_t)sizeof(dns_sortaddr_t), (void *)sortaddr);
	return;
}


__private_extern__
void
_dns_resolver_set_domain(dns_create_resolver_t *_resolver, const char *domain)
{
	_dns_resolver_add_attribute(_resolver, RESOLVER_ATTRIBUTE_DOMAIN, (uint32_t)strlen(domain) + 1, (void *)domain);
	return;
}


__private_extern__
void
_dns_resolver_set_flags(dns_create_resolver_t *_resolver, uint32_t flags)
{
	_dns_resolver_buf_t	*resolver	= (_dns_resolver_buf_t *)*_resolver;

	resolver->resolver.flags = htonl(flags);
	return;
}


__private_extern__
void
_dns_resolver_set_if_index(dns_create_resolver_t *_resolver, uint32_t if_index)
{
	_dns_resolver_buf_t	*resolver	= (_dns_resolver_buf_t *)*_resolver;

	resolver->resolver.if_index = htonl(if_index);
	return;
}


__private_extern__
void
_dns_resolver_set_options(dns_create_resolver_t *_resolver, const char *options)
{
	_dns_resolver_add_attribute(_resolver, RESOLVER_ATTRIBUTE_OPTIONS, (uint32_t)strlen(options) + 1, (void *)options);
	return;
}


__private_extern__
void
_dns_resolver_set_order(dns_create_resolver_t *_resolver, uint32_t order)
{
	_dns_resolver_buf_t	*resolver	= (_dns_resolver_buf_t *)*_resolver;

	resolver->resolver.search_order = htonl(order);
	return;
}


__private_extern__
void
_dns_resolver_set_port(dns_create_resolver_t *_resolver, uint16_t port)
{
	_dns_resolver_buf_t	*resolver	= (_dns_resolver_buf_t *)*_resolver;

	resolver->resolver.port = htons(port);
	return;
}


/*
 * rankReachability()
 *   Not reachable       == 0
 *   Connection Required == 1
 *   Reachable           == 2
 */
static int
rankReachability(SCNetworkReachabilityFlags flags)
{
	int	rank = 0;

	if (flags & kSCNetworkReachabilityFlagsReachable)		rank = 2;
	if (flags & kSCNetworkReachabilityFlagsConnectionRequired)	rank = 1;
	return rank;
}


static void
_dns_resolver_set_reach_flags(dns_create_resolver_t _resolver)
{
	_dns_resolver_buf_t	*resolver	= (_dns_resolver_buf_t *)_resolver;

	if (resolver->resolver.n_nameserver != 0) {
		dns_attribute_t			*attribute;
		SCNetworkReachabilityFlags	flags		= kSCNetworkReachabilityFlagsReachable;
		uint32_t			n_attribute;
		uint32_t			n_nameserver	= 0;
		CFMutableDictionaryRef		targetOptions;

		targetOptions = CFDictionaryCreateMutable(NULL,
							  0,
							  &kCFTypeDictionaryKeyCallBacks,
							  &kCFTypeDictionaryValueCallBacks);
		CFDictionarySetValue(targetOptions,
				     kSCNetworkReachabilityOptionServerBypass,
				     kCFBooleanTrue);
		if (resolver->resolver.if_index != 0) {
			char		if_name[IFNAMSIZ];

			if (if_indextoname(ntohl(resolver->resolver.if_index), if_name) != NULL) {
				CFStringRef	targetInterface;

				targetInterface = CFStringCreateWithCString(NULL,
									    if_name,
									    kCFStringEncodingASCII);
				CFDictionarySetValue(targetOptions,
						     kSCNetworkReachabilityOptionInterface,
						     targetInterface);
				CFRelease(targetInterface);
			}
		}

		attribute   = (dns_attribute_t *)(void *)&resolver->attribute[0];
		n_attribute = ntohl(resolver->n_attribute);

		while (n_attribute >= sizeof(dns_attribute_t)) {
			uint32_t	attribute_length	= ntohl(attribute->length);
			uint32_t	attribute_type		= ntohl(attribute->type);

			if (attribute_type == RESOLVER_ATTRIBUTE_ADDRESS) {
				struct sockaddr			*addr;
				CFDataRef			addrData;
				SCNetworkReachabilityFlags	ns_flags;
				Boolean				ok;
				SCNetworkReachabilityRef	target;

				addr = (struct sockaddr *)&attribute->attribute[0];
				addrData = CFDataCreate(NULL, (const UInt8 *)addr, addr->sa_len);
				CFDictionarySetValue(targetOptions,
						     kSCNetworkReachabilityOptionRemoteAddress,
						     addrData);
				CFRelease(addrData);

				target = SCNetworkReachabilityCreateWithOptions(NULL, targetOptions);
				if (target == NULL) {
					CFDictionaryRemoveValue(targetOptions, kSCNetworkReachabilityOptionInterface);
					target = SCNetworkReachabilityCreateWithOptions(NULL, targetOptions);
					if (target != NULL) {
						// if interface name not (no longer) valid
						CFRelease(target);
						flags = 0;
						break;
					}

					// address not valid?
					my_log(LOG_ERR,
					       "_dns_resolver_set_reach_flags SCNetworkReachabilityCreateWithOptions() failed:\n  options = %@",
					       targetOptions);
					break;
				}

				ok = SCNetworkReachabilityGetFlags(target, &ns_flags);
				CFRelease(target);
				if (!ok) {
					break;
				}

				if ((n_nameserver++ == 0) ||
				    (rankReachability(ns_flags) < rankReachability(flags))) {
					/* return the first (and later, worst case) result */
					flags = ns_flags;
				}
			}

			attribute   = (dns_attribute_t *)((void *)attribute + attribute_length);
			n_attribute -= attribute_length;
		}

		CFRelease(targetOptions);

		resolver->resolver.reach_flags = htonl(flags);
	}

	return;
}


__private_extern__
void
_dns_resolver_set_timeout(dns_create_resolver_t *_resolver, uint32_t timeout)
{
	_dns_resolver_buf_t	*resolver	= (_dns_resolver_buf_t *)*_resolver;

	resolver->resolver.timeout = htonl(timeout);
	return;
}


__private_extern__
void
_dns_resolver_set_service_identifier(dns_create_resolver_t *_resolver, uint32_t service_identifier)
{
	_dns_resolver_buf_t *resolver	= (_dns_resolver_buf_t *)*_resolver;

	resolver->resolver.service_identifier = htonl(service_identifier);
}


__private_extern__
void
_dns_resolver_free(dns_create_resolver_t *_resolver)
{
	_dns_resolver_buf_t	*resolver	= (_dns_resolver_buf_t *)*_resolver;

	free(resolver);
	*_resolver = NULL;
	return;
}
