/*
 * Copyright (c) 2011-2014 Apple Inc. All rights reserved.
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


#include <pthread.h>
#include <notify.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <dispatch/dispatch.h>
#include <xpc/xpc.h>

#include "libSystemConfiguration_client.h"
#include "network_information.h"
#include "network_information_priv.h"

static nwi_state_t	G_nwi_state		= NULL;
static pthread_mutex_t	nwi_store_lock		= PTHREAD_MUTEX_INITIALIZER;
static boolean_t	nwi_store_token_valid	= FALSE;

static pthread_once_t	initialized		= PTHREAD_ONCE_INIT;
static int		nwi_store_token;

static boolean_t	nwi_store_force_refresh	= FALSE;

#pragma mark -
#pragma mark Network information [nwi] client support


// Note: protected by __nwi_configuration_queue()
static int			nwi_active	= 0;
static libSC_info_client_t	*nwi_client	= NULL;


static dispatch_queue_t
__nwi_configuration_queue()
{
	static dispatch_once_t  once;
	static dispatch_queue_t q;

	dispatch_once(&once, ^{
		q = dispatch_queue_create(NWI_SERVICE_NAME, NULL);
	});

	return q;
}

static
void
_nwi_state_initialize(void)
{
	const char	*nwi_key	= nwi_state_get_notify_key();
	uint32_t	status		= notify_register_check(nwi_key,
								&nwi_store_token);

	if (status != NOTIFY_STATUS_OK) {
		fprintf(stderr, "nwi_state: registration failed (%u)\n", status);
	}
	else {
		nwi_store_token_valid = TRUE;
	}
}

static
void
_nwi_set_alias(nwi_state* state, nwi_ifstate* ifstate)
{
	nwi_ifstate*	  ifstate_alias;
	int af = ifstate->af;
	int af_alias;

	af_alias = (af == AF_INET)?AF_INET6:AF_INET;

	ifstate_alias =
		nwi_state_get_ifstate_with_name(state, af_alias,
					ifstate->ifname);

	if (ifstate_alias != NULL) {
		ifstate_alias->af_alias = ifstate;
	}
	ifstate->af_alias = ifstate_alias;
	return;
}

static
void
_nwi_state_reset_alias(nwi_state_t state) {
	int i;

	for (i = 0; i < state->ipv4_count; i++) {
		state->nwi_ifstates[i].af_alias = NULL;
	}

	for (i = state->ipv6_start;
	     i < state->ipv6_start + state->ipv6_count; i++) {
		_nwi_set_alias(state, &state->nwi_ifstates[i]);
	}
}


#pragma mark -
#pragma mark Network information [nwi] APIs


/*
 * Function: nwi_state_get_notify_key
 * Purpose:
 *   Returns the BSD notify key to use to monitor when the state changes.
 *
 * Note:
 *   The nwi_state_copy API uses this notify key to monitor when the state
 *   changes, so each invocation of nwi_state_copy returns the current
 *   information.
 */
const char *
nwi_state_get_notify_key()
{
#if	!TARGET_IPHONE_SIMULATOR
	return "com.apple.system.SystemConfiguration.nwi";
#else	// !TARGET_IPHONE_SIMULATOR
	return "com.apple.iOS_Simulator.SystemConfiguration.nwi";
#endif	// !TARGET_IPHONE_SIMULATOR
}

#define ATOMIC_CMPXCHG(p, o, n)	__sync_bool_compare_and_swap((p), (o), (n))
#define ATOMIC_INC(p)		__sync_fetch_and_add((p), 1)		// return (n++);
#define ATOMIC_DEC(p)		__sync_sub_and_fetch((p), 1)		// return (--n);

void
_nwi_state_force_refresh()
{
	ATOMIC_CMPXCHG(&nwi_store_force_refresh, FALSE, TRUE);
}

static void
nwi_state_retain(nwi_state_t state)
{
	ATOMIC_INC(&state->ref);
	return;
}

/*
 * Function: nwi_state_release
 * Purpose:
 *   Release the memory associated with the network state.
 */
void
nwi_state_release(nwi_state_t state)
{
	if (ATOMIC_DEC(&state->ref) > 0) {
		// if not last reference
		return;
	}

	// release connection reference on 1-->0 transition
	if (state->svr) {
		dispatch_sync(__nwi_configuration_queue(), ^{
			if (--nwi_active == 0) {
				// if last reference, drop connection
				libSC_info_client_release(nwi_client);
				nwi_client = NULL;
			}
		});
	}

	// release nwi_state
	free(state);

	return;
}

static nwi_state *
_nwi_state_copy_data()
{
	nwi_state_t		nwi_state	= NULL;
	static const char	*proc_name	= NULL;
	xpc_object_t		reqdict;
	xpc_object_t		reply;

	dispatch_sync(__nwi_configuration_queue(), ^{
		if ((nwi_active++ == 0) || (nwi_client == NULL)) {
			static dispatch_once_t	once;
			static const char	*service_name	= NWI_SERVICE_NAME;

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

			nwi_client =
				libSC_info_client_create(__nwi_configuration_queue(),	// dispatch queue
							 service_name,			// XPC service name
							 "Network information");	// service description
			if (nwi_client == NULL) {
				--nwi_active;
			}
		}
	});

	if ((nwi_client == NULL) || !nwi_client->active) {
		// if network information server not available
		return NULL;
	}

	// create message
	reqdict = xpc_dictionary_create(NULL, NULL, 0);

	// set process name
	if (proc_name != NULL) {
		xpc_dictionary_set_string(reqdict, NWI_PROC_NAME, proc_name);
	}

	// set request
	xpc_dictionary_set_int64(reqdict, NWI_REQUEST, NWI_REQUEST_COPY);

	// send request to the DNS configuration server
	reply = libSC_send_message_with_reply_sync(nwi_client, reqdict);
	xpc_release(reqdict);

	if (reply != NULL) {
		const void	*dataRef;
		size_t		dataLen	= 0;

		dataRef = xpc_dictionary_get_data(reply, NWI_CONFIGURATION, &dataLen);
		if (dataRef != NULL) {
			nwi_state = malloc(dataLen);
			bcopy((void *)dataRef, nwi_state, dataLen);
			nwi_state->ref = 0;
			nwi_state->svr = TRUE;
		}

		xpc_release(reply);
	}

	return nwi_state;
}

/*
 * Function: nwi_state_copy
 * Purpose:
 *   Returns the current network state information.
 *   Release after use by calling nwi_state_release().
 */
nwi_state_t
nwi_state_copy(void)
{
	boolean_t	force_refresh;
	nwi_state_t     nwi_state = NULL;
	nwi_state_t	old_state = NULL;

	pthread_once(&initialized, _nwi_state_initialize);
	pthread_mutex_lock(&nwi_store_lock);

	force_refresh = ATOMIC_CMPXCHG(&nwi_store_force_refresh, TRUE, FALSE);

	if (G_nwi_state != NULL) {
		int		check = 0;
		uint32_t	status;

		if (nwi_store_token_valid == FALSE) {
			/* have to throw cached copy away every time */
			check = 1;
		}
		else {
			status = notify_check(nwi_store_token, &check);
			if (status != NOTIFY_STATUS_OK) {
				fprintf(stderr, "nwi notify_check: failed with %u\n",
					status);
				/* assume that it changed, throw cached copy away */
				check = 1;
			}
		}
		if (check != 0 || force_refresh) {
			/* new need snapshot */
			old_state = G_nwi_state;
			G_nwi_state = NULL;
		}
	}
	/* Let's populate the cache if it's empty */
	if (G_nwi_state == NULL) {
		G_nwi_state = _nwi_state_copy_data();
		if (G_nwi_state != NULL) {
			/* one reference for G_nwi_state */
			nwi_state_retain(G_nwi_state);
			_nwi_state_reset_alias(G_nwi_state);
		}
	}
	if (G_nwi_state != NULL) {
		/* another reference for this caller */
		nwi_state_retain(G_nwi_state);
	}
	nwi_state = G_nwi_state;
	pthread_mutex_unlock(&nwi_store_lock);

	if (old_state != NULL) {
		/* get rid of G_nwi_state reference */
		nwi_state_release(old_state);
	}
	return nwi_state;
}

/*
 * Function: _nwi_state_ack
 * Purpose:
 *   Acknowledge receipt and any changes associated with the [new or
 *   updated] network state.
 */
void
_nwi_state_ack(nwi_state_t state, const char *bundle_id)
{
	xpc_object_t	reqdict;

	if (state == NULL) {
		return;	// ASSERT
	}

	if ((nwi_client == NULL) || !nwi_client->active) {
		// if network information server not available
		return;
	}

	dispatch_sync(__nwi_configuration_queue(), ^{
		nwi_active++;	// keep connection active (for the life of the process)
	});

	// create message
	reqdict = xpc_dictionary_create(NULL, NULL, 0);

	// set request
	xpc_dictionary_set_int64(reqdict, NWI_REQUEST, NWI_REQUEST_ACKNOWLEDGE);

	// set generation
	xpc_dictionary_set_uint64(reqdict, NWI_GENERATION, state->generation_count);

	// send acknowledgement to the DNS configuration server
	xpc_connection_send_message(nwi_client->connection, reqdict);

	xpc_release(reqdict);
	return;
}

/*
 * Function: nwi_state_get_generation
 * Purpose:
 *   Returns the generation (mach_time) of the nwi_state data.
 *   Every time the data is updated due to changes
 *   in the network, this value will change.
 */
uint64_t
nwi_state_get_generation(nwi_state_t state)
{
	return (state->generation_count);
}

/*
 * Function: nwi_ifstate_get_generation
 * Purpose:
 *   Returns the generation (mach_time) of the nwi_ifstate data.
 */
uint64_t
nwi_ifstate_get_generation(nwi_ifstate_t ifstate)
{
	return (ifstate->if_generation_count);
}

/*
 * Function: nwi_ifstate_get_ifname
 * Purpose:
 *   Return the interface name of the specified ifstate.
 */
const char *
nwi_ifstate_get_ifname(nwi_ifstate_t ifstate)
{
	return (ifstate != NULL?ifstate->ifname:NULL);

}

static uint64_t
flags_from_af(int af)
{
    return ((af == AF_INET)
	    ? NWI_IFSTATE_FLAGS_HAS_IPV4
	    : NWI_IFSTATE_FLAGS_HAS_IPV6);
}
/*
 * Function: nwi_ifstate_get_flags
 * Purpose:
 *   Return the flags for the given ifstate (see above for bit definitions).
 */
nwi_ifstate_flags
nwi_ifstate_get_flags(nwi_ifstate_t ifstate)
{
	nwi_ifstate_t		alias = ifstate->af_alias;
	nwi_ifstate_flags 	flags = 0ULL;

	flags |= flags_from_af(ifstate->af);
	if ((ifstate->flags & NWI_IFSTATE_FLAGS_HAS_DNS) != 0) {
		flags |= NWI_IFSTATE_FLAGS_HAS_DNS;

	}
	if (alias != NULL) {
		flags |= flags_from_af(alias->af);
		if ((alias->flags & NWI_IFSTATE_FLAGS_HAS_DNS) != 0) {
			flags |= NWI_IFSTATE_FLAGS_HAS_DNS;
		}
	}
	return flags;
}

/*
 * Function: nwi_state_get_first_ifstate
 * Purpose:
 *   Returns the first and highest priority interface that has connectivity
 *   for the specified address family 'af'. 'af' is either AF_INET or AF_INET6.
 *   The connectivity provided is for general networking.   To get information
 *   about an interface that isn't available for general networking, use
 *   nwi_state_get_ifstate().
 *
 *   Use nwi_ifstate_get_next() to get the next, lower priority interface
 *   in the list.
 *
 *   Returns NULL if no connectivity for the specified address family is
 *   available.
 */
nwi_ifstate_t
nwi_state_get_first_ifstate(nwi_state_t state, int af)
{
	nwi_ifstate_t ifstate;

	if (state == NULL) {
		return NULL;
	}

	ifstate = nwi_state_get_ifstate_with_index(state, af, 0);
	if ((ifstate->flags & NWI_IFSTATE_FLAGS_NOT_IN_LIST)
	    != 0) {
		ifstate =  NULL;
	}

	return ifstate;

}

/*
 * Function: nwi_state_get_ifstate
 * Purpose:
 *   Return information for the specified interface 'ifname'.
 *
 *   This API directly returns the ifstate for the specified interface.
 *   This is the only way to access information about an interface that isn't
 *   available for general networking.
 *
 *   Returns NULL if no information is available for that interface.
 */
nwi_ifstate_t
nwi_state_get_ifstate(nwi_state_t state, const char * ifname)
{
	nwi_ifstate_t ifstate = nwi_state_get_ifstate_with_name(state, AF_INET, ifname);
	if (ifstate == NULL) {
		ifstate = nwi_state_get_ifstate_with_name(state, AF_INET6, ifname);
	}
	return ifstate;

}

/*
 * Function: nwi_ifstate_get_next
 * Purpose:
 *   Returns the next, lower priority nwi_ifstate_t after the specified
 *   'ifstate' for the protocol family 'af'.
 *
 *   Returns NULL when the end of the list is reached.
 */
nwi_ifstate_t
nwi_ifstate_get_next(nwi_ifstate_t ifstate, int af)
{
	nwi_ifstate_t alias, next;

	alias =
		(af == ifstate->af)?ifstate:ifstate->af_alias;

	if (alias == NULL) {
		return NULL;
	}

	/* We don't return interfaces marked rank never */
	if ((alias->flags & NWI_IFSTATE_FLAGS_NOT_IN_LIST) != 0) {
		return NULL;
	}

	next = ++alias;

	if ((next->flags & NWI_IFSTATE_FLAGS_NOT_IN_LIST) == 0) {
		return next;
	}
	return NULL;
}

/*
 * Function: nwi_ifstate_compare_rank
 * Purpose:
 *   Compare the relative rank of two nwi_ifstate_t objects.
 *
 *   The "rank" indicates the importance of the underlying interface.
 *
 * Returns:
 *   0 	if ifstate1 and ifstate2 are ranked equally
 *  -1	if ifstate1 is ranked ahead of ifstate2
 *   1	if ifstate2 is ranked ahead of ifstate1
 */
int
nwi_ifstate_compare_rank(nwi_ifstate_t ifstate1, nwi_ifstate_t ifstate2)
{
	return RankCompare(ifstate1->rank, ifstate2->rank);
}

/*
 * nwi_state_get_reachability_flags
 *
 * returns the global reachability flags for a given address family.
 * If no address family is passed in, it returns the global reachability
 * flags for either families.
 *
 * The reachability flags returned follow the definition of
 * SCNetworkReachabilityFlags.
 *
 * If the flags are zero (i.e. do not contain kSCNetworkReachabilityFlagsReachable), there is no connectivity.
 *
 * Otherwise, at least kSCNetworkReachabilityFlagsReachable is set:
 *        Reachable only
 *          No other connection flags are set.
 *        Reachable and no ConnectionRequired
 *          If we have connectivity for the specified address family (and we'd
 *          be returning the reachability flags associated with the default route)
 *        Reachable and ConnectionRequired
 *          If we do not currently have an active/primary network but we may
 *          be able to establish connectivity.
 *        Reachable and OnDemand
 *          If we do not currently have an active/primary network but we may
 *          be able to establish connective on demand.
 *        Reachable and TransientConnection
 *          This connection is transient.
 *        Reachable and WWAN
 *          This connection will be going over the cellular network.
 */
uint32_t
nwi_state_get_reachability_flags(nwi_state_t nwi_state, int af)
{
	if (nwi_state == NULL) {
		return (0);
	}
	if (af == AF_INET || af == AF_INET6) {
		nwi_ifstate_t ifstate;

		ifstate = nwi_state_get_first_ifstate(nwi_state, af);

		if (ifstate != NULL) {
			return ifstate->reach_flags;
		}

		return (af == AF_INET) ? nwi_state->reach_flags_v4 : nwi_state->reach_flags_v6;
	} else {
		nwi_ifstate_t ifstate_v4;
		nwi_ifstate_t ifstate_v6;

		ifstate_v4 = nwi_state_get_first_ifstate(nwi_state, AF_INET);
		ifstate_v6 = nwi_state_get_first_ifstate(nwi_state, AF_INET6);

		if (ifstate_v4 != NULL) {
			if (ifstate_v6 != NULL) {
				if (nwi_ifstate_compare_rank(ifstate_v4, ifstate_v6) > 0) {
					return ifstate_v6->reach_flags;
				} else {
					return ifstate_v4->reach_flags;
				}
			} else {
				return ifstate_v4->reach_flags;
			}
		} else {
			if (ifstate_v6 != NULL) {
				return ifstate_v6->reach_flags;
			}
		}

		if (nwi_state->reach_flags_v4 != 0) {
			return nwi_state->reach_flags_v4;
		}
		// This is the case where both ifstate are NULL.
		return nwi_state->reach_flags_v6;
	}
}

/*
 * nwi_ifstate_get_vpn_server
 *
 * returns a sockaddr representation of the vpn server address.
 * NULL if PPP/VPN/IPSec server address does not exist.
 */
const struct sockaddr *
nwi_ifstate_get_vpn_server(nwi_ifstate_t ifstate)
{
	const struct sockaddr * vpn_server_addr;

	vpn_server_addr = (const struct sockaddr *)(void *)
			   &ifstate->vpn_server_address;

	if (vpn_server_addr->sa_family == 0) {
		return NULL;
	}
	return vpn_server_addr;
}

/*
 * nwi_ifstate_get_reachability_flags
 *
 * returns the reachability flags for the interface given an address family.
 * The flags returned are those determined outside of
 * the routing table.  [None, ConnectionRequired, OnDemand,
 * Transient Connection, WWAN].
 */
uint32_t
nwi_ifstate_get_reachability_flags(nwi_ifstate_t ifstate)
{
	return ifstate->reach_flags;
}

/*
 * nwi_ifstate_get_signature
 *
 * returns the signature and its length for an ifstate given an address family.
 * If AF_UNSPEC is passed in, the signature for a given ifstate is returned.
 *
 * If the signature does not exist, NULL is returned.
 */
const uint8_t *
nwi_ifstate_get_signature(nwi_ifstate_t ifstate, int af, int * length)
{
	nwi_ifstate_t i_state = NULL;

	switch (af) {
		case AF_UNSPEC:
			i_state = ifstate;
			break;
		case AF_INET:
		case AF_INET6:
			i_state = (ifstate->af == af) ? ifstate : ifstate->af_alias;
			break;
		default:
			break;

	}

	if (i_state != NULL) {
		if ((i_state->flags & NWI_IFSTATE_FLAGS_HAS_SIGNATURE) != 0) {
			*length = sizeof(i_state->signature);
			return (i_state->signature);
		}
	}

	*length = 0;
	return NULL;
}

static inline
boolean_t
_nwi_ifstate_is_in_list(nwi_ifstate_t ifstate, int af)
{
	nwi_ifstate_t i_state;

	i_state = (ifstate->af == af) ? ifstate : ifstate->af_alias;
	if (i_state == NULL) {
		return FALSE;
	}

	if ((nwi_ifstate_get_flags(i_state) & NWI_IFSTATE_FLAGS_NOT_IN_LIST) == 0) {
		return TRUE;
	}

	return FALSE;
}

/*
 * nwi_ifstate_get_dns_signature
 *
 * returns the signature and its length for given
 * ifstate with a valid dns configuration.
 *
 * If the signature does not exist, NULL is returned.
 *
 */
const uint8_t *
nwi_ifstate_get_dns_signature(nwi_ifstate_t ifstate, int * length)
{
	const uint8_t *	signature = NULL;
	const uint8_t *	v4_signature;
	int		v4_signature_len;
	const uint8_t *	v6_signature;
	int		v6_signature_len;

	*length = 0;

	if ((nwi_ifstate_get_flags(ifstate) & NWI_IFSTATE_FLAGS_HAS_DNS) == 0) {
		return NULL;
	}

	v4_signature = nwi_ifstate_get_signature(ifstate, AF_INET, &v4_signature_len);
	v6_signature = nwi_ifstate_get_signature(ifstate, AF_INET6, &v6_signature_len);
	if (v4_signature == NULL && v6_signature == NULL) {
		return NULL;
	}

	if (_nwi_ifstate_is_in_list(ifstate, AF_INET) == TRUE) {
		signature = v4_signature;
		*length = v4_signature_len;
	} else {
		if (_nwi_ifstate_is_in_list(ifstate, AF_INET6) != TRUE && v4_signature_len > 0) {
			/* v6 is ranked never, v4 is ranked never but has a valid signature */
			signature = v4_signature;
			*length = v4_signature_len;
		} else {
			/* v6 is not ranked never or v4 has no signature */
			signature = v6_signature;
			*length = v6_signature_len;
		}
	}

	return signature;
}


#pragma mark -
#pragma mark Network information [nwi] test code


#ifdef MAIN

int
main(int argc, char **argv)
{
	dns_config_t	*config;

	config = dns_configuration_copy();
	if (config != NULL) {
		dns_configuration_free(&config);
	}

	exit(0);
}

#endif
