/*
 * Copyright (c) 2002-2014 Apple Inc. All rights reserved.
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
 * - split code out from eventmon.c
 */


#ifndef _EV_IPV4_H
#define _EV_IPV4_H

#include <TargetConditionals.h>
#include <netinet/in_var.h>

__BEGIN_DECLS

void	ipv4_interface_update(struct ifaddrs *ifap, const char *if_name);

void	ipv4_arp_collision(const char *if_name,
			   struct in_addr ip_addr,
			   int hw_len, const void * hw_addr);

#if	!TARGET_OS_IPHONE
void	ipv4_port_in_use(uint16_t port, pid_t req_pid);
#endif	/* !TARGET_OS_IPHONE */

void	ipv4_router_arp_failure(const char * if_name);
void	ipv4_router_arp_alive(const char * if_name);

__END_DECLS

#endif /* _EV_IPV4_H */

