/*
 * Copyright (c) 2004, 2006, 2008-2013 Apple Inc. All rights reserved.
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

/*
 * Modification History
 *
 * March 9, 2004		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <dispatch/dispatch.h>
#include <xpc/xpc.h>

#include "libSystemConfiguration_client.h"
#include "dnsinfo.h"
#include "dnsinfo_private.h"

typedef uint32_t getflags;

static boolean_t
add_list(void **padding, uint32_t *n_padding, int32_t count, int32_t size, void **list)
{
	int32_t	need;

	need = count * size;
	if (need > *n_padding) {
		return FALSE;
	}

	*list = (need == 0) ? NULL : *padding;
	*padding   += need;
	*n_padding -= need;
	return TRUE;
}


#define	DNS_CONFIG_BUF_MAX	1024*1024


static dns_resolver_t *
expand_resolver(_dns_resolver_buf_t *buf, uint32_t n_buf, void **padding, uint32_t *n_padding)
{
	dns_attribute_t		*attribute;
	uint32_t		n_attribute;
	int32_t			n_nameserver    = 0;
	int32_t			n_search	= 0;
	int32_t			n_sortaddr      = 0;
	dns_resolver_t		*resolver	= (dns_resolver_t *)&buf->resolver;

	if (n_buf < sizeof(_dns_resolver_buf_t)) {
		goto error;
	}

	// initialize domain

	resolver->domain = NULL;

	// initialize nameserver list

	resolver->n_nameserver = ntohl(resolver->n_nameserver);
	if (!add_list(padding,
		      n_padding,
		      resolver->n_nameserver,
		      sizeof(DNS_PTR(struct sockaddr *, x)),
		      (void **)&resolver->nameserver)) {
		goto error;
	}

	// initialize port

	resolver->port = ntohs(resolver->port);

	// initialize search list

	resolver->n_search = ntohl(resolver->n_search);
	if (!add_list(padding,
		      n_padding,
		      resolver->n_search,
		      sizeof(DNS_PTR(char *, x)),
		      (void **)&resolver->search)) {
		goto error;
	}

	// initialize sortaddr list

	resolver->n_sortaddr = ntohl(resolver->n_sortaddr);
	if (!add_list(padding,
		      n_padding,
		      resolver->n_sortaddr,
		      sizeof(DNS_PTR(dns_sortaddr_t *, x)),
		      (void **)&resolver->sortaddr)) {
		goto error;
	}

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
				resolver->nameserver[n_nameserver++] = (struct sockaddr *)&attribute->attribute[0];
				break;

			case RESOLVER_ATTRIBUTE_SEARCH :
				resolver->search[n_search++] = (char *)&attribute->attribute[0];
				break;

			case RESOLVER_ATTRIBUTE_SORTADDR :
				resolver->sortaddr[n_sortaddr++] = (dns_sortaddr_t *)(void *)&attribute->attribute[0];
				break;

			case RESOLVER_ATTRIBUTE_OPTIONS :
				resolver->options = (char *)&attribute->attribute[0];
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


static dns_config_t *
expand_config(_dns_config_buf_t *buf)
{
	dns_attribute_t		*attribute;
	dns_config_t		*config			= (dns_config_t *)buf;
	uint32_t		n_attribute;
	uint32_t		n_padding;
	int32_t			n_resolver		= 0;
	int32_t			n_scoped_resolver	= 0;
	int32_t			n_service_specific_resolver	= 0;
	void			*padding;

	// establish padding

	padding   = &buf->attribute[ntohl(buf->n_attribute)];
	n_padding = ntohl(buf->n_padding);

	// initialize resolver lists

	config->n_resolver = ntohl(config->n_resolver);
	if (!add_list(&padding,
		      &n_padding,
		      config->n_resolver,
		      sizeof(DNS_PTR(dns_resolver_t *, x)),
		      (void **)&config->resolver)) {
		goto error;
	}

	config->n_scoped_resolver = ntohl(config->n_scoped_resolver);
	if (!add_list(&padding,
		      &n_padding,
		      config->n_scoped_resolver,
		      sizeof(DNS_PTR(dns_resolver_t *, x)),
		      (void **)&config->scoped_resolver)) {
		goto error;
	}

	config->n_service_specific_resolver = ntohl(config->n_service_specific_resolver);
	if (!add_list(&padding,
		      &n_padding,
		      config->n_service_specific_resolver,
		      sizeof(DNS_PTR(dns_resolver_t *, x)),
		      (void **)&config->service_specific_resolver)) {
		goto error;
	}

	// process configuration buffer "attribute" data

	n_attribute = ntohl(buf->n_attribute);
	attribute   = (dns_attribute_t *)(void *)&buf->attribute[0];

	while (n_attribute >= sizeof(dns_attribute_t)) {
		uint32_t	attribute_length	= ntohl(attribute->length);
		uint32_t	attribute_type		= ntohl(attribute->type);

		switch (attribute_type) {
			case CONFIG_ATTRIBUTE_RESOLVER :
			case CONFIG_ATTRIBUTE_SCOPED_RESOLVER   :
			case CONFIG_ATTRIBUTE_SERVICE_SPECIFIC_RESOLVER : {
				dns_resolver_t	*resolver;

				// expand resolver buffer

				resolver = expand_resolver((_dns_resolver_buf_t *)(void *)&attribute->attribute[0],
							   attribute_length - sizeof(dns_attribute_t),
							   &padding,
							   &n_padding);
				if (resolver == NULL) {
					goto error;
				}

				// add resolver to config list

				if (attribute_type == CONFIG_ATTRIBUTE_RESOLVER) {
					config->resolver[n_resolver++] = resolver;
				} else if (attribute_type == CONFIG_ATTRIBUTE_SCOPED_RESOLVER) {
					config->scoped_resolver[n_scoped_resolver++] = resolver;
				} else if (attribute_type == CONFIG_ATTRIBUTE_SERVICE_SPECIFIC_RESOLVER) {
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


const char *
dns_configuration_notify_key()
{
	const char	*key;

#if	!TARGET_IPHONE_SIMULATOR
	key = "com.apple.system.SystemConfiguration.dns_configuration";
#else	// !TARGET_IPHONE_SIMULATOR
	key = "com.apple.iOS_Simulator.SystemConfiguration.dns_configuration";
#endif	// !TARGET_IPHONE_SIMULATOR
	return key;
}


#pragma mark -
#pragma mark DNS configuration [dnsinfo] client support


// Note: protected by __dns_configuration_queue()
static int			dnsinfo_active	= 0;
static libSC_info_client_t	*dnsinfo_client	= NULL;


static dispatch_queue_t
__dns_configuration_queue()
{
	static dispatch_once_t  once;
	static dispatch_queue_t q;

	dispatch_once(&once, ^{
		q = dispatch_queue_create(DNSINFO_SERVICE_NAME, NULL);
	});

	return q;
}


dns_config_t *
dns_configuration_copy()
{
	uint8_t			*buf		= NULL;
	dns_config_t		*config		= NULL;
	static const char	*proc_name	= NULL;
	xpc_object_t		reqdict;
	xpc_object_t		reply;

	dispatch_sync(__dns_configuration_queue(), ^{
		if ((dnsinfo_active++ == 0) || (dnsinfo_client == NULL)) {
			static dispatch_once_t	once;
			static const char	*service_name	= DNSINFO_SERVICE_NAME;

			dispatch_once(&once, ^{
				const char	*name;

				// get [XPC] service name
				name = getenv(service_name);
				if ((name != NULL) && (issetugid() == 0)) {
					service_name = strdup(name);
				}

				// get process name
				proc_name = getprogname();
			});

			dnsinfo_client =
				libSC_info_client_create(__dns_configuration_queue(),	// dispatch queue
							 service_name,			// XPC service name
							 "DNS configuration");		// service description
			if (dnsinfo_client == NULL) {
				--dnsinfo_active;
			}
		}
	});

	if ((dnsinfo_client == NULL) || !dnsinfo_client->active) {
		// if DNS configuration server not available
		return NULL;
	}

	// create message
	reqdict = xpc_dictionary_create(NULL, NULL, 0);

	// set process name
	if (proc_name != NULL) {
		xpc_dictionary_set_string(reqdict, DNSINFO_PROC_NAME, proc_name);
	}

	// set request
	xpc_dictionary_set_int64(reqdict, DNSINFO_REQUEST, DNSINFO_REQUEST_COPY);

	// send request to the DNS configuration server
	reply = libSC_send_message_with_reply_sync(dnsinfo_client, reqdict);
	xpc_release(reqdict);

	if (reply != NULL) {
		const void	*dataRef;
		size_t		dataLen	= 0;

		dataRef = xpc_dictionary_get_data(reply, DNSINFO_CONFIGURATION, &dataLen);
		if ((dataRef != NULL) &&
		    ((dataLen >= sizeof(_dns_config_buf_t)) && (dataLen <= DNS_CONFIG_BUF_MAX))) {
			_dns_config_buf_t       *config         = (_dns_config_buf_t *)(void *)dataRef;
			uint32_t                n_padding       = ntohl(config->n_padding);

			if (n_padding <= (DNS_CONFIG_BUF_MAX - dataLen)) {
				size_t        len;

				len = dataLen + n_padding;
				buf = malloc(len);
				bcopy((void *)dataRef, buf, dataLen);
				bzero(&buf[dataLen], n_padding);
			}
		}

		xpc_release(reply);
	}

	if (buf != NULL) {
		/* ALIGN: cast okay since _dns_config_buf_t is int aligned */
		config = expand_config((_dns_config_buf_t *)(void *)buf);
		if (config == NULL) {
			free(buf);
		}
	}

	return config;
}


void
dns_configuration_free(dns_config_t *config)
{
	if (config == NULL) {
		return;	// ASSERT
	}

	dispatch_sync(__dns_configuration_queue(), ^{
		if (--dnsinfo_active == 0) {
			// if last reference, drop connection
			libSC_info_client_release(dnsinfo_client);
			dnsinfo_client = NULL;
		}
	});

	free((void *)config);
	return;
}


void
_dns_configuration_ack(dns_config_t *config, const char *bundle_id)
{
	xpc_object_t	reqdict;

	if (config == NULL) {
		return;	// ASSERT
	}

	if ((dnsinfo_client == NULL) || !dnsinfo_client->active) {
		// if DNS configuration server not available
		return;
	}

	dispatch_sync(__dns_configuration_queue(), ^{
		dnsinfo_active++;	// keep connection active (for the life of the process)
	});

	// create message
	reqdict = xpc_dictionary_create(NULL, NULL, 0);

	// set request
	xpc_dictionary_set_int64(reqdict, DNSINFO_REQUEST, DNSINFO_REQUEST_ACKNOWLEDGE);

	// set generation
	xpc_dictionary_set_uint64(reqdict, DNSINFO_GENERATION, config->generation);

	// send acknowledgement to the DNS configuration server
	xpc_connection_send_message(dnsinfo_client->connection, reqdict);

	xpc_release(reqdict);
	return;
}

#ifdef MAIN

int
main(int argc, char **argv)
{
	dns_config_t	*config;

	config = dns_configuration_copy();
	if (config != NULL) {
		dns_configuration_free(config);
	}

	exit(0);
}

#endif
