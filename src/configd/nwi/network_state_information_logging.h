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

#ifndef _NETWORK_STATE_INFORMATION_LOGGING_H
#define _NETWORK_STATE_INFORMATION_LOGGING_H

#include <os/availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>

#include <network_information.h>
#include "network_state_information_priv.c"

__BEGIN_DECLS

#ifndef	my_log
#define	my_log(__level, __format, ...)	SC_log(__level, __format, ## __VA_ARGS__)
#define	MY_LOG_DEFINED_LOCALLY
#endif	// !my_log

#ifndef	my_log_context_type
#define	my_log_context_type	void *
#define	MY_LOG_CONTEXT_TYPE_DEFINED_LOCALLY
#endif	// !my_log_context_type

#ifndef	my_log_context_name
#define	my_log_context_name	context
#define	MY_LOG_CONTEXT_NAME_DEFINED_LOCALLY
#endif	// !my_log_context_name

#include "SCNetworkReachabilityLogging.h"

/*
 * _nwi_ifstate_flags_str()
 *
 * Returns a string representation of the nwi_state flags.
 *	"(IPv4,IPv6,DNS,...)"
 */
static __inline__ void
_nwi_ifstate_flags_str(nwi_ifstate_flags flags, char *str, size_t len)
{
	size_t			n;
	nwi_ifstate_flags	remaining;

	assert(len >= sizeof("(0x01234567)"));		// check min buffer size

	flags &= NWI_IFSTATE_FLAGS_MASK;
	if (flags == 0) {
		str[0] = '\0';
		return;
	}

	len--;	// leave room for the closing paren
	n = strlcpy(str, "(", len);
	remaining = flags;

	if ((remaining & NWI_IFSTATE_FLAGS_HAS_IPV4) &&
	    (n < len) && ((len - n) > sizeof("IPv4,"))) {
		n = strlcat(str, "IPv4,", len);
		remaining &= ~NWI_IFSTATE_FLAGS_HAS_IPV4;
	}

	if ((remaining & NWI_IFSTATE_FLAGS_HAS_IPV6) &&
	    (n < len) && ((len - n) > sizeof("IPv6,"))) {
		n = strlcat(str, "IPv6,", len);
		remaining &= ~NWI_IFSTATE_FLAGS_HAS_IPV6;
	}

	if ((remaining & NWI_IFSTATE_FLAGS_HAS_DNS) &&
	    (n < len) && ((len - n) > sizeof("DNS,"))) {
		n = strlcat(str, "DNS,", len);
		remaining &= ~NWI_IFSTATE_FLAGS_HAS_DNS;
	}

	if ((remaining & NWI_IFSTATE_FLAGS_HAS_CLAT46) &&
	    (n < len) && ((len - n) > sizeof("CLAT46,"))) {
		n = strlcat(str, "CLAT46,", len);
		remaining &= ~NWI_IFSTATE_FLAGS_HAS_CLAT46;
	}

	if ((remaining & NWI_IFSTATE_FLAGS_NOT_IN_LIST) &&
	    (n < len) && ((len - n) > sizeof("NOT-IN-LIST,"))) {
		n = strlcat(str, "NOT-IN-LIST,", len);
		remaining &= ~NWI_IFSTATE_FLAGS_NOT_IN_LIST;
	}

	if ((remaining & NWI_IFSTATE_FLAGS_HAS_SIGNATURE) &&
	    (n < len) && ((len - n) > sizeof("SIGNATURE,"))) {
		n = strlcat(str, "SIGNATURE,", len);
		remaining &= ~NWI_IFSTATE_FLAGS_HAS_SIGNATURE;
	}

	if ((remaining & NWI_IFSTATE_FLAGS_NOT_IN_IFLIST) &&
	    (n < len) && ((len - n) > sizeof("NOT-IN-IFLIST,"))) {
		n = strlcat(str, "NOT-IN-IFLIST,", len);
		remaining &= ~NWI_IFSTATE_FLAGS_NOT_IN_IFLIST;
	}

	if (remaining != 0) {
		if ((n >= len) ||
		    ((len - n) <= sizeof("0x01234567,"))) {
			// if we don't have enough space, truncate and start over
			n = strlcpy(str, "(", len);
			remaining = flags;
		}

		n += snprintf(str + n, len - n, ",%p", (void *)remaining);
	}

	if (n-- > 0) {
		str[n] = ')';	// trailing "," --> ")"
	}

	return;
}

static __inline__ const char *
_nwi_ifstate_rank_str(Rank rank)
{
	const char	*str	= "???";

	switch (RANK_ASSERTION_MASK(rank)) {
	    case kRankAssertionFirst:
		str = "First";
		break;
	    case kRankAssertionDefault:
		str = "Default";
		break;
	    case kRankAssertionLast:
		str = "Last";
		break;
	    case kRankAssertionNever:
		str = "Never";
		break;
	    case kRankAssertionScoped:
		str = "Scoped";
		break;
	    default:
		str = "???";
		break;
	}

	return str;
}

static __inline__ void
_nwi_ifstate_log(nwi_ifstate_t ifstate, boolean_t debug, my_log_context_type my_log_context_name)
{
#if	defined(MY_LOG_CONTEXT_TYPE_DEFINED_LOCALLY) && defined(MY_LOG_CONTEXT_NAME_DEFINED_LOCALLY)
#pragma	unused(my_log_context_name)
#endif
	union {
		const void		*bytes;
		const struct in_addr	*ia;
		const struct in6_addr	*ia6;
	} addr;
	char				addr_str[INET6_ADDRSTRLEN];
	nwi_ifstate_flags		flags_ifstate;
	char				flags_str[100];
	const char			*if_name;
	SCNetworkReachabilityFlags	reach_flags;
	char				reach_str[100];
	const struct sockaddr		*vpn_addr;

	// nwi_ifstate flags
	flags_ifstate = nwi_ifstate_get_flags(ifstate);
	if (debug) {
		flags_ifstate |= NWI_IFSTATE_FLAGS(ifstate->flags);
	}
	flags_ifstate &= ~NWI_IFSTATE_FLAGS_HAS_SIGNATURE;	// exclude flag ('cause we'll report the signature only if present)
	_nwi_ifstate_flags_str(flags_ifstate, flags_str, sizeof(flags_str));

	// if_name
	if_name = nwi_ifstate_get_ifname(ifstate);

	// reachability flags
	reach_flags = nwi_ifstate_get_reachability_flags(ifstate);

	// IP address
	addr.bytes = nwi_ifstate_get_address(ifstate);
	if (inet_ntop(ifstate->af, addr.bytes, addr_str, sizeof(addr_str)) == NULL) {
		strlcpy(addr_str, "???", sizeof(addr_str));
	}

	// verbose format (e.g. scutil)
	my_log(LOG_INFO, " %7s : flags      : %p %s",
	       if_name,
	       (void *)flags_ifstate,
	       flags_str);

	my_log(LOG_INFO, "           address    : %s", addr_str);

	// VPN server address
	vpn_addr = nwi_ifstate_get_vpn_server(ifstate);
	if (vpn_addr != NULL) {
		char		vpn_str[INET6_ADDRSTRLEN];

		_SC_sockaddr_to_string(vpn_addr, vpn_str, sizeof(vpn_str));
		my_log(LOG_INFO, "           VPN server : %s", vpn_str);
	}

	// reachability
	__SCNetworkReachability_flags_string(reach_flags, TRUE, reach_str, sizeof(reach_str));
	my_log(LOG_INFO, "           reach      : %s", reach_str);

	if (debug) {
		uint64_t	generation;
		Rank		rank;
		uint32_t	rank_index;
		const char	*rank_str;
		const uint8_t	*signature;
		int		signature_length	= 0;

		// Rank
		rank = ifstate->rank;
		rank_str = _nwi_ifstate_rank_str(rank);
		rank_index = RANK_INDEX_MASK(rank);
		if (rank_index != kRankIndexMask) {
			my_log(LOG_INFO, "           rank       : 0x%08x (%s, %u)", rank, rank_str, rank_index);
		} else {
			my_log(LOG_INFO, "           rank       : 0x%08x (%s, Last)", rank, rank_str);
		}

		// signature
		signature = nwi_ifstate_get_signature(ifstate, AF_UNSPEC, &signature_length);
		if (signature != NULL) {
			CFDataRef	digest;

			digest = CFDataCreate(NULL, signature, signature_length);
			my_log(LOG_INFO, "           signature  : %@", digest);
			CFRelease(digest);
		}

		// generation
		generation = nwi_ifstate_get_generation(ifstate);
		my_log(LOG_INFO, "           generation : %llu", generation);
	}

	return;
}

static __inline__ void
_nwi_state_reachability_log(nwi_state_t state, boolean_t debug, int af, my_log_context_type my_log_context_name)
{
#if	defined(MY_LOG_CONTEXT_TYPE_DEFINED_LOCALLY) && defined(MY_LOG_CONTEXT_NAME_DEFINED_LOCALLY)
#pragma	unused(my_log_context_name)
#endif
	uint32_t	flags;
	char		flags_str[100];

	flags = nwi_state_get_reachability_flags(state, af);
	__SCNetworkReachability_flags_string(flags, TRUE, flags_str, sizeof(flags_str));
	if (!debug) {
		my_log(LOG_INFO, "%s", "");
	}
	my_log(LOG_INFO, "   REACH : flags %s", flags_str);

	return;
}

static __inline__ void
_nwi_state_log(nwi_state_t state, boolean_t debug, my_log_context_type my_log_context_name)
{
#if	defined(MY_LOG_CONTEXT_TYPE_DEFINED_LOCALLY) && defined(MY_LOG_CONTEXT_NAME_DEFINED_LOCALLY)
#pragma	unused(my_log_context_name)
#endif
	unsigned int	count;
	nwi_ifindex_t	i;
	nwi_ifstate_t	ifstate;

	if (!debug) {
		my_log(LOG_INFO, "%s", "Network information");
	} else {
		my_log(LOG_INFO,
		       "Network information (generation %llu size=%lu)",
		       nwi_state_get_generation(state),
		       nwi_state_size(state));
	}

	// report IPv4 state
	if (!debug) {
		// show regular interfaces
		my_log(LOG_INFO, "%s", "");
		my_log(LOG_INFO, "%s", "IPv4 network interface information");
		ifstate = nwi_state_get_first_ifstate(state, AF_INET);
		if (ifstate == NULL) {
			my_log(LOG_INFO, "%s", "   No IPv4 states found");
		} else {
			while (ifstate != NULL) {
				_nwi_ifstate_log(ifstate, debug, my_log_context_name);
				ifstate = nwi_ifstate_get_next(ifstate, AF_INET);
			}
		}
	} else {
		my_log(LOG_INFO, "%s", "IPv4 network interface information");
		if (state->ipv4_count > 0) {
			// show ALL interfaces
			for (i = 0, ifstate = nwi_state_ifstate_list(state, AF_INET);
			     i < state->ipv4_count; i++, ifstate++) {
				_nwi_ifstate_log(ifstate, debug, my_log_context_name);
			}
		} else {
			my_log(LOG_INFO, "%s", "   No IPv4 states found");
		}
	}
	_nwi_state_reachability_log(state, debug, AF_INET, my_log_context_name);

	// report IPv6 state
	if (!debug) {
		// show regular interfaces
		my_log(LOG_INFO, "%s", "");
		my_log(LOG_INFO, "%s", "IPv6 network interface information");
		ifstate = nwi_state_get_first_ifstate(state, AF_INET6);
		if (ifstate == NULL) {
			my_log(LOG_INFO, "%s", "   No IPv6 states found\n");
		} else {
			while (ifstate != NULL) {
				_nwi_ifstate_log(ifstate, debug, my_log_context_name);
				ifstate = nwi_ifstate_get_next(ifstate, AF_INET6);
			}
		}
	} else {
		my_log(LOG_INFO, "%s", "IPv6 network interface information");
		if (state->ipv6_count > 0) {
			// show ALL interfaces
			for (i = 0, ifstate = nwi_state_ifstate_list(state, AF_INET6);
			     i < state->ipv6_count; i++, ifstate++) {
				_nwi_ifstate_log(ifstate, debug, my_log_context_name);
			}
		} else {
			my_log(LOG_INFO, "%s", "   No IPv6 states found\n");
		}
	}
	_nwi_state_reachability_log(state, debug, AF_INET6, my_log_context_name);

	count = nwi_state_get_interface_names(state, NULL, 0);
	if (count > 0) {
		const char	*names[count];

		count = nwi_state_get_interface_names(state, names, count);
		if (count > 0) {
			char	str[count * (IFNAMSIZ + 1)];

			memset(str, 0, sizeof(str));
			for (unsigned int i = 0; i < count; i++) {
				if (i != 0) {
					strlcat(str, " ", sizeof(str));
				}
				strlcat(str, names[i], sizeof(str));
			}

			if (!debug) {
				my_log(LOG_INFO, "%s", "");
			}
			my_log(LOG_INFO, "Network interfaces: %s", str);
		}
	}

	return;
}

#ifdef	MY_LOG_DEFINED_LOCALLY
#undef	my_log
#undef	MY_LOG_DEFINED_LOCALLY
#endif	// MY_LOG_DEFINED_LOCALLY

#ifdef	MY_LOG_CONTEXT_TYPE_DEFINED_LOCALLY
#undef	my_log_context_type
#undef	MY_LOG_CONTEXT_TYPE_DEFINED_LOCALLY
#endif	// MY_LOG_CONTEXT_TYPE_DEFINED_LOCALLY

#ifdef	MY_LOG_CONTEXT_NAME_DEFINED_LOCALLY
#undef	my_log_context_name
#undef	MY_LOG_CONTEXT_NAME_DEFINED_LOCALLY
#endif	// MY_LOG_CONTEXT_NAME_DEFINED_LOCALLY

__END_DECLS

#endif	// _NETWORK_STATE_INFORMATION_LOGGING_H
