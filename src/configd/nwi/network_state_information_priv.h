/*
 * Copyright (c) 2011-2013, 2016-2019 Apple Inc. All rights reserved.
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

#ifndef _NETWORK_STATE_INFORMATION_PRIV_H_
#define _NETWORK_STATE_INFORMATION_PRIV_H_

#include <CommonCrypto/CommonDigest.h>
#include <net/if.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "network_information.h"

#define NWI_STATE_VERSION	((uint32_t)0x20170601)


#define NWI_IFSTATE_FLAGS_NOT_IN_LIST	0x0008
#define NWI_IFSTATE_FLAGS_HAS_SIGNATURE	0x0010
#define NWI_IFSTATE_FLAGS_NOT_IN_IFLIST	0x0020

/*
 * NWI_IFSTATE_FLAGS_MASK
 * - these are the bits that get preserved, all others are
 *   control (last item, diff)
 */
#define NWI_IFSTATE_FLAGS_MASK		0x00ff


#define NWI_IFSTATE_FLAGS_DIFF_MASK	0x0f00
#define NWI_IFSTATE_FLAGS_LAST_ITEM	0x1000

typedef enum {
    knwi_ifstate_difference_none = 0,
    knwi_ifstate_difference_changed = 1,
    knwi_ifstate_difference_removed = 2
} nwi_ifstate_difference_t;


/*
 * Type: Rank
 * Purpose:
 *   A 32-bit value to encode the relative rank of a service.
 *
 *   The top 8 bits are used to hold the rank assertion (first, default, last,
 *   never, scoped);
 *
 *   The bottom 24 bits are used to store the service index (i.e. the
 *   position within the service order array).
 */
typedef uint32_t        Rank;
#define RANK_ASSERTION_MAKE(r)		((Rank)(r) << 24)		// rank assertion (top 8 bits)
#define kRankAssertionFirst		RANK_ASSERTION_MAKE(0)
#define kRankAssertionDefault		RANK_ASSERTION_MAKE(1)
#define kRankAssertionLast		RANK_ASSERTION_MAKE(2)
#define kRankAssertionNever		RANK_ASSERTION_MAKE(3)
#define kRankAssertionScoped		RANK_ASSERTION_MAKE(4)
#define kRankAssertionMask		RANK_ASSERTION_MAKE(0xff)
#define RANK_ASSERTION_MASK(r)		((Rank)(r) & kRankAssertionMask)
#define RANK_INDEX_MAKE(r)		((Rank)(r))			// rank index (bottom 24 bits)
#define kRankIndexMask			RANK_INDEX_MAKE(0xffffff)
#define RANK_INDEX_MASK(r)		((Rank)(r) & kRankIndexMask)

typedef int32_t		nwi_ifindex_t;

#define NWI_SIGNATURE_LENGTH	20

#pragma pack(4)
typedef struct _nwi_ifstate {
	char			ifname[IFNAMSIZ];
	uint64_t		flags;
	nwi_ifindex_t		af_alias_offset; /* relative index to alias */
	Rank			rank;
	sa_family_t		af;
	union {
	    struct in_addr	iaddr;
	    struct in6_addr	iaddr6;
	};
	uint64_t		if_generation_count;
	uint32_t		reach_flags;
	union {
	    struct sockaddr_in	vpn_server_address4;
	    struct sockaddr_in6	vpn_server_address6;
	} vpn_server_address;
	unsigned char		signature[NWI_SIGNATURE_LENGTH];
} nwi_ifstate;
#pragma pack()

#pragma pack(4)
typedef struct _nwi_state {
	uint32_t	version;	/* NWI_STATE_VERSION */
	nwi_ifindex_t	max_if_count;	/* available slots per protocol */
	nwi_ifindex_t	ipv4_count;	/* # of v4 ifstates in use */
	nwi_ifindex_t	ipv6_count;	/* # of v6 ifstates in use */
	nwi_ifindex_t	if_list_count;	/* # of if_list[] slots in use */
	uint32_t	ref;		/* reference count */
	uint32_t	reach_flags_v4;
	uint32_t	reach_flags_v6;
	uint64_t	generation_count;
	nwi_ifstate	ifstate_list[1];/* (max_if_count * 2) ifstates */
/*	nwi_ifindex_t 	if_list[0];        max_if_count indices */
} nwi_state;
#pragma pack()

static __inline__ int
nwi_other_af(int af)
{
	return ((af == AF_INET) ? (AF_INET6) : (AF_INET));
}

static __inline__ size_t
nwi_state_compute_size(unsigned int max_if_count)
{
	size_t	size;

	size = offsetof(nwi_state, ifstate_list[max_if_count * 2])
		+ sizeof(nwi_ifindex_t) * max_if_count;
	return (size);
}

static __inline__ size_t
nwi_state_size(nwi_state_t state)
{
	return (nwi_state_compute_size(state->max_if_count));
}

static __inline__ nwi_ifstate_t
nwi_state_ifstate_list(nwi_state_t state, int af)
{
	if (af == AF_INET) {
		return (state->ifstate_list);
	}
	return (state->ifstate_list + state->max_if_count);
}

static __inline__ nwi_ifindex_t *
nwi_state_if_list(nwi_state_t state)
{
	return ((nwi_ifindex_t *)&state->ifstate_list[state->max_if_count * 2]);
}

static __inline__ int
uint32_cmp(uint32_t a, uint32_t b)
{
	int		ret;

	if (a == b) {
		ret = 0;
	}
	else if (a < b) {
		ret = -1;
	}
	else {
		ret = 1;
	}
	return (ret);
}

static __inline__ int
RankCompare(Rank a, Rank b)
{
	return (uint32_cmp(a, b));
}

/*
 * Function: nwi_state_get_ifstate_count
 * Purpose:
 *   Return the number of ifstate elements for the specified address family
 *   'af'. 'af' is either AF_INET or AF_INET6.
 *
 *   Returns zero if there are no elements.
 */
static __inline__
int
nwi_state_get_ifstate_count(nwi_state_t state, int af)
{
	return (af == AF_INET)?state->ipv4_count:state->ipv6_count;
}

static __inline__ nwi_ifstate_t
nwi_ifstate_get_alias(nwi_ifstate_t ifstate, int af)
{
	if (ifstate->af == af) {
		return (ifstate);
	}
	if (ifstate->af_alias_offset == 0) {
		return (NULL);
	}
	return (ifstate + ifstate->af_alias_offset);
}

/*
 *   The ifstate list is sorted in order of decreasing priority, with the
 *   highest priority element appearing at index zero.
 *
 *   If 'idx' is outside of the bounds of the corresponding array, returns NULL.
 */
static __inline__
nwi_ifstate_t
nwi_state_get_ifstate_with_index(nwi_state_t state, int af, int idx)
{
	int i_idx = idx;

	if (idx >= nwi_state_get_ifstate_count(state, af)) {
		return (NULL);
	}

	if (af == AF_INET6) {
		i_idx = idx + state->max_if_count;
	}

	return &state->ifstate_list[i_idx];
}

/*
 * Function: nwi_state_get_ifstate_with_name
 * Purpose:
 *   Return the ifstate for the specified ifstate for the specified address
 *   family 'af'. 'af' is either AF_INET or AF_INET6.
 *
 *   Returns NULL if no such information exists.
 */
static __inline__
nwi_ifstate_t
nwi_state_get_ifstate_with_name(nwi_state_t state,
				 int af, const char * name)
{
	int idx = 0;
	int count;
	nwi_ifstate_t ifstate = NULL;

	if (state == NULL) {
		return NULL;
	}

	count = (af == AF_INET)
	?state->ipv4_count:state->ipv6_count;


	while (idx < count) {
		ifstate = nwi_state_get_ifstate_with_index(state, af, idx);
		if (ifstate == NULL) {
			break;
		}
		if (strcmp(name,
			   nwi_ifstate_get_ifname(ifstate)) == 0) {
			return (ifstate);
		}
		idx++;
	}
	return (NULL);
}

static __inline__
void
_nwi_ifstate_set_vpn_server(nwi_ifstate_t ifstate, struct sockaddr *serv_addr)
{
	size_t len;

	if (serv_addr == NULL) {
		memset(&ifstate->vpn_server_address, 0, sizeof(ifstate->vpn_server_address));
		return;
	}

	len = serv_addr->sa_len;

	if (len == 0 || len > sizeof(ifstate->vpn_server_address)) {
		return;
	}

	memcpy(&ifstate->vpn_server_address,
	       serv_addr,
	       len);
	return;

}

static __inline__
void
_nwi_state_set_reachability_flags(nwi_state_t state, uint32_t reach_flags_v4, uint32_t reach_flags_v6)
{
	state->reach_flags_v4 = reach_flags_v4;
	state->reach_flags_v6 = reach_flags_v6;
	return;
}

nwi_state_t
nwi_state_new(nwi_state_t old_state, int elems);

nwi_state_t
nwi_state_make_copy(nwi_state_t state);

static __inline__ void
nwi_state_free(nwi_state_t state)
{
	free(state);
	return;
}

void
nwi_state_finalize(nwi_state_t state);

nwi_ifstate_t
nwi_state_add_ifstate(nwi_state_t state, const char* ifname, int af,
		      uint64_t flags, Rank rank,
		      void * ifa, struct sockaddr * vpn_server_addr, uint32_t reach_flags);

void
nwi_ifstate_set_signature(nwi_ifstate_t ifstate, uint8_t * signature);

void
nwi_state_clear(nwi_state_t state, int af);

nwi_state_t
nwi_state_diff(nwi_state_t old_state, nwi_state_t new_state);

void *
nwi_ifstate_get_address(nwi_ifstate_t ifstate);

const char *
nwi_ifstate_get_diff_str(nwi_ifstate_t ifstate);

nwi_ifstate_difference_t
nwi_ifstate_get_difference(nwi_ifstate_t diff_ifstate);

void
_nwi_state_update_interface_generations(nwi_state_t old_state, nwi_state_t state, nwi_state_t changes);

void
_nwi_state_compute_sha256_hash(nwi_state_t state,
			       unsigned char hash[CC_SHA256_DIGEST_LENGTH]);

#endif	// _NETWORK_STATE_INFORMATION_PRIV_H_
