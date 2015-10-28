/*
 * Copyright (c) 2003-2006 Apple Computer, Inc. All rights reserved.
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

#ifndef __DNS_H__
#define __DNS_H__

#include <sys/cdefs.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/socket.h>

/*
 * The functions described in this access layer support multiple DNS
 * client configurations.  Each DNS client has its own set of nameserver
 * addresses and its own set of operational parameters.  Each client
 * can perform DNS queries and searches independent of other clients.
 * Each client has a symbolic name which is of the same format as a
 * domain name, e.g. "apple.com".  A special meta-client, known as the
 * "Super" DNS client, acts as a router for DNS queries.  The Super
 * client chooses among all available clients by finding a best match
 * between the domain name given in a query and the names of all known
 * clients.
 *
 * The configuration for a particular client may be read from a file
 * having the same format as the traditional "/etc/resolv.conf" file.
 * However, client configurations are not limited to being stored in
 * files.  The implementation of the library may also locate client
 * configuratins in other data sources, such as the System Configuration
 * Database.  Users of this API should make no assumptions about the
 * source of the configuration data.
 */

typedef const struct __dns_handle_private_struct *dns_handle_t;


__BEGIN_DECLS

/*
 * Create a client handle for DNS access.
 *
 * "Super" DNS client
 *
 * dns_open(NULL) returns a "super" client that routes DNS queries
 * among all DNS configurations known to the system.
 *
 * Queries for qualified names are sent using a client configuration that
 * best matches the domain name given in the query. For example, if there
 * is a client named "apple.com", a search for "foo.apple.com" would use the
 * resolver configuration specified for that client.  The matching algorithm
 * chooses the client with the maximum number of matching domain components.
 * For example, if there are clients named "a.b.c", and "b.c", a search for
 * "x.a.b.c" would use the "a.b.c" resolver configuration, while a search
 * for "x.y.b.c" would use the "b.c" client.  If there are no matches, the 
 * configuration settings in the default client - generally corresponding to
 * the /etc/resolv.conf file or to the "primary" DNS configuration on the
 * system are used for the query.
 *
 * The domain search list defined in the "default" client is used to search
 * for unqualified names, by appending each domain in the search list and
 * then following the logic for matching qualified names described above.
 *
 * The DNS access APIs may be used by multiple threads.  Each thread must
 * use a separate DNS client handle created by calling dns_open().
 *
 * A simple DNS client handle may be obtained by providing a non-NULL value
 * for the "name" parameter.  Simple clients correspond to a single DNS
 * configuration, derived from a resolv.conf format file or from some other
 * source of configurations known to the system.
 * The name parameter may be a full or relative path name (starting with '/'
 * or '.'), in which case the client's configuration is read from the
 * named file.  If the name parameter is not a file path name, the routine
 * will search through all known sources of DNS configuration data on the
 * system to locate DNS configuration data corresponding to the name supplied,
 * or NULL if none can be found.
 *
 * Use _PATH_RESCONF to open /etc/resolv.conf.
 */
extern dns_handle_t dns_open(const char *name);

/*
 * Close a client and free its resources.
 */
extern void dns_free(dns_handle_t dns);

/*
 * Enable / Disable debug logging.
 */
extern void dns_set_debug(dns_handle_t dns, uint32_t flag);

/*
 * Returns the number of names in the search list.
 */
extern uint32_t dns_search_list_domain_count(dns_handle_t dns);

/* 
 * Returns the domain name at index i in the search list.
 * Returns NULL if there are no names in the search list,
 * or if i is out of range.
 * Caller must free the returned value.
 */
extern char *dns_search_list_domain(dns_handle_t dns, uint32_t i);

/*
 * Resolve a name.
 * The name is considered fully-qualified (the search list is not used).
 * Caller must supply buffer space for the reply message and the server address.
 */
extern int32_t dns_query(dns_handle_t dns, const char *name, uint32_t dnsclass, uint32_t dnstype, char *buf, uint32_t len, struct sockaddr *from, uint32_t *fromlen);

/*
 * Search for a name.
 * Caller must supply buffer space for the reply message and the server address.
 */
extern int32_t dns_search(dns_handle_t dns, const char *name, uint32_t dnsclass, uint32_t dnstype, char *buf, uint32_t len, struct sockaddr *from, uint32_t *fromlen);

__END_DECLS

#endif /* __DNS_H__ */
