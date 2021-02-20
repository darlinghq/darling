/*
 * Copyright (c) 2013, 2015-2018 Apple Inc. All rights reserved.
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

#include <os/availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <SystemConfiguration/SCPrivate.h>	// for SC_log
#include <arpa/inet.h>

#include <dnsinfo.h>
#include "dnsinfo_private.h"

#define	DNS_CONFIG_BUF_MAX	1024*1024

__BEGIN_DECLS

#ifndef	my_log
#define	my_log(__level, __format, ...)	SC_log(__level, __format, ## __VA_ARGS__)
#define	MY_LOG_DEFINED_LOCALLY
#endif	// !my_log

/*
 * claim space for a list [of pointers] from the expanded DNS configuration padding
 */
static __inline__ boolean_t
__dns_configuration_expand_add_list(void **padding, uint32_t *n_padding, uint32_t count, uint32_t size, void **list)
{
	uint32_t	need;

	need = count * size;
	if (need > *n_padding) {
		return FALSE;
	}

	*list = (need == 0) ? NULL : *padding;
	*padding   += need;
	*n_padding -= need;
	return TRUE;
}


/*
 * expand a DNS "resolver" from the provided buffer
 */
static __inline__ dns_resolver_t *
_dns_configuration_expand_resolver(_dns_resolver_buf_t *buf, uint32_t n_buf, void **padding, uint32_t *n_padding)
{
	dns_attribute_t		*attribute;
	uint32_t		n_attribute;
	int32_t			n_nameserver    = 0;
	int32_t			n_search	= 0;
	int32_t			n_sortaddr      = 0;
	void			*ptr;
	dns_resolver_t		*resolver	= (dns_resolver_t *)&buf->resolver;

	if (n_buf < sizeof(_dns_resolver_buf_t)) {
		goto error;
	}

	// initialize domain

	resolver->domain = NULL;

	// initialize nameserver list

	resolver->n_nameserver = ntohl(resolver->n_nameserver);
	if (!__dns_configuration_expand_add_list(padding,
						 n_padding,
						 resolver->n_nameserver,
						 sizeof(DNS_PTR(struct sockaddr *, x)),
						 &ptr)) {
		goto error;
	}
	resolver->nameserver = ptr;

	// initialize port

	resolver->port = ntohs(resolver->port);

	// initialize search list

	resolver->n_search = ntohl(resolver->n_search);
	if (!__dns_configuration_expand_add_list(padding,
						 n_padding,
						 resolver->n_search,
						 sizeof(DNS_PTR(char *, x)),
						 &ptr)) {
		goto error;
	}
	resolver->search = ptr;

	// initialize sortaddr list

	resolver->n_sortaddr = ntohl(resolver->n_sortaddr);
	if (!__dns_configuration_expand_add_list(padding,
						 n_padding,
						 resolver->n_sortaddr,
						 sizeof(DNS_PTR(dns_sortaddr_t *, x)),
						 &ptr)) {
		goto error;
	}
	resolver->sortaddr = ptr;

	// initialize options

	resolver->options = NULL;

	// initialize timeout

	resolver->timeout = ntohl(resolver->timeout);

	// initialize search_order

	resolver->search_order = ntohl(resolver->search_order);

	// initialize if_index

	resolver->if_index = ntohl(resolver->if_index);

	// initialize service_identifier

	resolver->service_identifier = ntohl(resolver->service_identifier);

	// initialize flags

	resolver->flags = ntohl(resolver->flags);

	// initialize SCNetworkReachability flags

	resolver->reach_flags = ntohl(resolver->reach_flags);

	// process resolver buffer "attribute" data

	n_attribute = n_buf - sizeof(_dns_resolver_buf_t);
	/* ALIGN: alignment not assumed, using accessors */
	attribute = (dns_attribute_t *)(void *)&buf->attribute[0];
	if (n_attribute != ntohl(buf->n_attribute)) {
		goto error;
	}

	while (n_attribute >= sizeof(dns_attribute_t)) {
		uint32_t	attribute_length	= ntohl(attribute->length);

		switch (ntohl(attribute->type)) {
			case RESOLVER_ATTRIBUTE_DOMAIN :
				resolver->domain = (char *)&attribute->attribute[0];
				break;

			case RESOLVER_ATTRIBUTE_ADDRESS :
				if (resolver->nameserver == NULL) {
					goto error;
				}
				resolver->nameserver[n_nameserver++] = (struct sockaddr *)&attribute->attribute[0];
				break;

			case RESOLVER_ATTRIBUTE_SEARCH :
				if (resolver->search == NULL) {
					goto error;
				}
				resolver->search[n_search++] = (char *)&attribute->attribute[0];
				break;

			case RESOLVER_ATTRIBUTE_SORTADDR :
				if (resolver->sortaddr == NULL) {
					goto error;
				}
				resolver->sortaddr[n_sortaddr++] = (dns_sortaddr_t *)(void *)&attribute->attribute[0];
				break;

			case RESOLVER_ATTRIBUTE_OPTIONS :
				resolver->options = (char *)&attribute->attribute[0];
				break;

			case RESOLVER_ATTRIBUTE_CONFIGURATION_ID :
				resolver->cid = (char *)&attribute->attribute[0];
				break;

			case RESOLVER_ATTRIBUTE_INTERFACE_NAME :
				resolver->if_name = (char *)&attribute->attribute[0];
				break;

			default :
				break;
		}

		attribute   = (dns_attribute_t *)((void *)attribute + attribute_length);
		n_attribute -= attribute_length;
	}

	if ((n_nameserver != resolver->n_nameserver) ||
	    (n_search     != resolver->n_search    ) ||
	    (n_sortaddr   != resolver->n_sortaddr  )) {
		goto error;
	}

	return resolver;

    error :

	return NULL;
}


/*
 * initialize a DNS "configuration" buffer
 */
static __inline__ _dns_config_buf_t *
_dns_configuration_buffer_create(const void *dataRef, size_t dataLen)
{
	uint8_t			*buf = NULL;
	size_t			bufLen;
	_dns_config_buf_t       *config         = (_dns_config_buf_t *)dataRef;
	size_t			configLen;
	uint32_t                n_attribute	= ntohl(config->n_attribute);
	uint32_t		n_padding       = ntohl(config->n_padding);

	/*
	 * Check that the size of the configuration header plus the size of the
	 * attribute data matches the size of the configuration buffer.
	 *
	 * If the sizes are different, something that should NEVER happen, CRASH!
	 */
	configLen = sizeof(_dns_config_buf_t) + n_attribute;
#ifdef	DEBUG
	assert(configLen == dataLen);
#else	// DEBUG
	if (configLen != dataLen) {
		my_log(LOG_ERR, "DNS configuration: size error (%zu != %zu)", configLen, dataLen);
		return NULL;
	}
#endif	// DEBUG

	/*
	 * Check that the size of the requested padding would not result in our
	 * allocating a configuration + padding buffer larger than our maximum size.
	 *
	 * If the requested padding size is too large, something that should NEVER
	 * happen, CRASH!
	 */
#ifdef	DEBUG
	assert(n_padding <= (DNS_CONFIG_BUF_MAX - dataLen));
#else	// DEBUG
	if (n_padding > (DNS_CONFIG_BUF_MAX - dataLen)) {
		my_log(LOG_ERR, "DNS configuration: padding error (%u > %zu)",
		       n_padding,
		       (DNS_CONFIG_BUF_MAX - dataLen));
		return NULL;
	}
#endif	// DEBUG

	/*
	 * Check that the actual size of the configuration data and any requested
	 * padding will be less than the maximum possible size of the in-memory
	 * configuration buffer.
	 *
	 * If the length needed is too large, something that should NEVER happen, CRASH!
	 */
	bufLen = dataLen + n_padding;
#ifdef	DEBUG
	assert(bufLen <= DNS_CONFIG_BUF_MAX);
#else	// DEBUG
	if (bufLen > DNS_CONFIG_BUF_MAX) {
		my_log(LOG_ERR, "DNS configuration: length error (%zu > %u)",
		       bufLen,
		       DNS_CONFIG_BUF_MAX);
		return NULL;
	}
#endif	// DEBUG

	// allocate a buffer large enough to hold both the configuration
	// data and the padding.
	buf = malloc(bufLen);
	memcpy(buf, (void *)dataRef, dataLen);
	memset(&buf[dataLen], 0, n_padding);

	return (_dns_config_buf_t *)(void *)buf;
}


/*
 * expand a DNS "configuration" from the provided buffer
 */
static __inline__ void
_dns_configuration_buffer_free(_dns_config_buf_t **buf)
{
	_dns_config_buf_t	*config	= (_dns_config_buf_t *)*buf;

	free(config);
	*buf = NULL;
	return;
}


/*
 * expand a DNS "configuration" from the provided buffer
 */
static __inline__ dns_config_t *
_dns_configuration_buffer_expand(_dns_config_buf_t *buf)
{
	dns_attribute_t		*attribute;
	dns_config_t		*config			= (dns_config_t *)buf;
	uint32_t		n_attribute;
	uint32_t		n_padding;
	int32_t			n_resolver		= 0;
	int32_t			n_scoped_resolver	= 0;
	int32_t			n_service_specific_resolver	= 0;
	void			*padding;
	void			*ptr;

	n_attribute = ntohl(buf->n_attribute);	// pre-validated (or known OK) at entry
	n_padding   = ntohl(buf->n_padding);	// pre-validated (or known OK) at entry

	// establish the start of padding to be after the last attribute

	padding = &buf->attribute[n_attribute];

	// initialize resolver lists

	config->n_resolver = ntohl(config->n_resolver);
	if (!__dns_configuration_expand_add_list(&padding,
						 &n_padding,
						 config->n_resolver,
						 sizeof(DNS_PTR(dns_resolver_t *, x)),
						 &ptr)) {
		goto error;
	}
	config->resolver = ptr;

	config->n_scoped_resolver = ntohl(config->n_scoped_resolver);
	if (!__dns_configuration_expand_add_list(&padding,
						 &n_padding,
						 config->n_scoped_resolver,
						 sizeof(DNS_PTR(dns_resolver_t *, x)),
						 &ptr)) {
		goto error;
	}
	config->scoped_resolver = ptr;

	config->n_service_specific_resolver = ntohl(config->n_service_specific_resolver);
	if (!__dns_configuration_expand_add_list(&padding,
						 &n_padding,
						 config->n_service_specific_resolver,
						 sizeof(DNS_PTR(dns_resolver_t *, x)),
						 &ptr)) {
		goto error;
	}
	config->service_specific_resolver = ptr;

	// process configuration buffer "attribute" data

	attribute = (dns_attribute_t *)(void *)&buf->attribute[0];

	while (n_attribute >= sizeof(dns_attribute_t)) {
		uint32_t	attribute_length	= ntohl(attribute->length);
		uint32_t	attribute_type		= ntohl(attribute->type);

		switch (attribute_type) {
			case CONFIG_ATTRIBUTE_RESOLVER :
			case CONFIG_ATTRIBUTE_SCOPED_RESOLVER   :
			case CONFIG_ATTRIBUTE_SERVICE_SPECIFIC_RESOLVER : {
				dns_resolver_t	*resolver;

				// expand resolver buffer

				resolver = _dns_configuration_expand_resolver((_dns_resolver_buf_t *)(void *)&attribute->attribute[0],
							   attribute_length - sizeof(dns_attribute_t),
							   &padding,
							   &n_padding);
				if (resolver == NULL) {
					goto error;
				}

				// add resolver to config list

				if (attribute_type == CONFIG_ATTRIBUTE_RESOLVER) {
					if (config->resolver == NULL) {
						goto error;
					}
					config->resolver[n_resolver++] = resolver;
				} else if (attribute_type == CONFIG_ATTRIBUTE_SCOPED_RESOLVER) {
					if (config->scoped_resolver == NULL) {
						goto error;
					}
					config->scoped_resolver[n_scoped_resolver++] = resolver;
				} else if (attribute_type == CONFIG_ATTRIBUTE_SERVICE_SPECIFIC_RESOLVER) {
					if (config->service_specific_resolver == NULL) {
						goto error;
					}
					config->service_specific_resolver[n_service_specific_resolver++] = resolver;
				}

				break;
			}

			default :
				break;
		}

		attribute   = (dns_attribute_t *)((void *)attribute + attribute_length);
		n_attribute -= attribute_length;
	}

	if (n_resolver != config->n_resolver) {
		goto error;
	}

	if (n_scoped_resolver != config->n_scoped_resolver) {
		goto error;
	}

	if (n_service_specific_resolver != config->n_service_specific_resolver) {
		goto error;
	}

	return config;

    error :

	return NULL;
}

#ifdef	MY_LOG_DEFINED_LOCALLY
#undef	my_log
#undef	MY_LOG_DEFINED_LOCALLY
#endif	// MY_LOG_DEFINED_LOCALLY

__END_DECLS

#endif	/* !_S_DNSINFO_INTERNAL_H */
