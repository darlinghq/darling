/*
 * Copyright (c) 2002, 2004, 2011, 2012, 2015, 2017 Apple Inc. All rights reserved.
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
 * August 5, 2002	Allan Nathanson <ajn@apple.com>
 * - initial revision
 */


#ifndef _EV_IPV6_H
#define _EV_IPV6_H

#include <netinet6/in6_var.h>

__BEGIN_DECLS

void	interface_update_ipv6(struct ifaddrs *ifap, const char *if_name);
void	ipv6_duplicated_address(const char * if_name, const struct in6_addr * addr,
				int hw_len, const void * hw_addr);
void	nat64_prefix_request(const char *if_name);
void	ipv6_router_expired(const char *if_name);

__END_DECLS

#endif /* _EV_IPV6_H */

