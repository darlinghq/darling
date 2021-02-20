/*
 * Copyright (c) 2017, 2018 Apple Inc. All rights reserved.
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

#ifndef _SCNETWORKREACHABILITYLOGGING_H
#define _SCNETWORKREACHABILITYLOGGING_H

#include <os/availability.h>
#include <TargetConditionals.h>
#include <assert.h>
#include <sys/cdefs.h>
#include <SystemConfiguration/SystemConfiguration.h>

__BEGIN_DECLS

/*
 * __SCNetworkReachability_flags_string()
 *
 * Returns a string representation of the SCNetworkReachability flags.
 *	debug==FALSE : " Reachable,Transient Connection,WWAN,..."
 *	debug==TRUE  : " 0x01234567 (Reachable,Transient Connection,WWAN,...)"
 */
static __inline__ void
__SCNetworkReachability_flags_string(SCNetworkReachabilityFlags flags, Boolean debug, char *str, size_t len)
{
	size_t				n;
	size_t				op;		// open paren
	SCNetworkReachabilityFlags	remaining;

	assert((len >= sizeof("Not Reachable,"            )) &&	// check min buffer size
	       (len >= sizeof("0x01234567 (Not Reachable)")) &&
	       (len >= sizeof("0x01234567 (0x01234567)"   )));

	if (!debug) {
		n = 0;
		str[n] = '\0';
	} else {
		n = snprintf(str, len, "0x%08x (", flags);
		len--;	// leave room for the closing paren
	}
	op = n;
	remaining = flags;

	if ((remaining == 0) &&
	    (n < len) && ((len - n) > sizeof("Not Reachable,"))) {
		n = strlcat(str, "Not Reachable,", len);
	}

	if ((remaining & kSCNetworkReachabilityFlagsReachable) &&
	    (n < len) && ((len - n) > sizeof("Reachable,"))) {
		n = strlcat(str, "Reachable,", len);
		remaining &= ~kSCNetworkReachabilityFlagsReachable;
	}

	if ((remaining & kSCNetworkReachabilityFlagsTransientConnection) &&
	    (n < len) && ((len - n) > sizeof("Transient Connection,"))) {
		n = strlcat(str, "Transient Connection,", len);
		remaining &= ~kSCNetworkReachabilityFlagsTransientConnection;
	}

	if ((remaining & kSCNetworkReachabilityFlagsConnectionRequired) &&
	    (n < len) && ((len - n) > sizeof("Connection Required,"))) {
		n = strlcat(str, "Connection Required,", len);
		remaining &= ~kSCNetworkReachabilityFlagsConnectionRequired;
	}

	if ((remaining & kSCNetworkReachabilityFlagsConnectionOnTraffic) &&
	    (n < len) && ((len - n) > sizeof("Automatic Connection On Traffic,"))) {
		n = strlcat(str, "Automatic Connection On Traffic,", len);
		remaining &= ~kSCNetworkReachabilityFlagsConnectionOnTraffic;
	}

	if ((remaining & kSCNetworkReachabilityFlagsConnectionOnDemand) &&
	    (n < len) && ((len - n) > sizeof("Automatic Connection On Demand,"))) {
		n = strlcat(str, "Automatic Connection On Demand,", len);
		remaining &= ~kSCNetworkReachabilityFlagsConnectionOnDemand;
	}

	if ((remaining & kSCNetworkReachabilityFlagsInterventionRequired) &&
	    (n < len) && ((len - n) > sizeof("Intervention Required,"))) {
		n = strlcat(str, "Intervention Required,", len);
		remaining &= ~kSCNetworkReachabilityFlagsInterventionRequired;
	}

	if ((remaining & kSCNetworkReachabilityFlagsIsLocalAddress) &&
	    (n < len) && ((len - n) > sizeof("Local Address,"))) {
		n = strlcat(str, "Local Address,", len);
		remaining &= ~kSCNetworkReachabilityFlagsIsLocalAddress;
	}

	if ((remaining & kSCNetworkReachabilityFlagsIsDirect) &&
	    (n < len) && ((len - n) > sizeof("Directly Reachable Address,"))) {
		n = strlcat(str, "Directly Reachable Address,", len);
		remaining &= ~kSCNetworkReachabilityFlagsIsDirect;
	}

#if	TARGET_OS_IPHONE
	if ((remaining & kSCNetworkReachabilityFlagsIsWWAN) &&
	    (n < len) && ((len - n) > sizeof("WWAN,"))) {
		n = strlcat(str, "WWAN,", len);
		remaining &= ~kSCNetworkReachabilityFlagsIsWWAN;
	}
#endif	// TARGET_OS_IPHONE

	if (remaining != 0) {
		if ((n >= len) ||
		    ((len - n) <= sizeof("0x01234567,"))) {
			// if we don't have enough space, truncate and start over
			str[op] = '\0';
			n = op;
			remaining = flags;
		}

		n += snprintf(str + n, len - n, "0x%08x,", remaining);
	}

	if (n-- > 0) {
		if (!debug) {
			str[n] = '\0';			// remove trailing ","
		} else {
			str[n] = ')';			// trailing "," --> ")"
		}
	}

	return;
}

__END_DECLS

#endif	/* _SCNETWORKREACHABILITYLOGGING_H */
