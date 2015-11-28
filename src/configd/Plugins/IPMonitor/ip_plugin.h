/*
 * Copyright (c) 2012-2014 Apple Inc.  All Rights Reserved.
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
 * ip_plugin.h
 * - header for IPMonitor.bundle
 */

#ifndef	_IP_PLUGIN_H
#define	_IP_PLUGIN_H

#include <netinet/in.h>
#include <SystemConfiguration/SCPrivate.h>
#include <net/if.h>

#ifdef TEST_IPV4_ROUTELIST
#define TEST_ROUTELIST
#endif

#ifdef TEST_IPV6_ROUTELIST
#define TEST_ROUTELIST
#endif

#ifndef countof
#define countof(array)	(sizeof(array) / sizeof((array)[0]))
#endif

#define kIsNULL				CFSTR("IsNULL")	/* CFBoolean */

#ifdef TEST_ROUTELIST
#define my_log(__level, fmt, ...)	SCPrint(TRUE, stdout, CFSTR(fmt "\n"), ## __VA_ARGS__)

#else /* TEST_ROUTELIST */

#define my_log(__level, fmt, ...)	SCLoggerLog(my_log_get_logger(), __level, CFSTR(fmt), ## __VA_ARGS__)
SCLoggerRef my_log_get_logger();

#endif /* TEST_ROUTELIST */

boolean_t
cfstring_to_ip(CFStringRef str, struct in_addr * ip_p);

boolean_t
cfstring_to_ip6(CFStringRef str, struct in6_addr * ip6_p);

unsigned int
my_if_nametoindex(const char * ifname);

const char *
my_if_indextoname(unsigned int idx, char if_name[IFNAMSIZ]);

boolean_t
service_contains_protocol(CFDictionaryRef service, int af);

CFDictionaryRef
ipv4_dict_create(CFDictionaryRef state_dict);

CFDictionaryRef
ipv6_dict_create(CFDictionaryRef state_dict);

#endif	// _IP_PLUGIN_H
