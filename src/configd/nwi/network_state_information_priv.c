/*
 * Copyright (c) 2011-2015, 2017, 2018 Apple Inc. All rights reserved.
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

#include <arpa/inet.h>
#include <assert.h>
#include <notify.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdbool.h>
#include "network_state_information_priv.h"
#include <limits.h>
#include <stdio.h>
#include <syslog.h>

#define NWI_IFSTATE_FLAGS(flags) \
	((flags) & NWI_IFSTATE_FLAGS_MASK)

#define NWI_IFSTATE_FLAGS_GET_DIFF(flags) \
	(((flags) & NWI_IFSTATE_FLAGS_DIFF_MASK) >> 8)

#define NWI_IFSTATE_FLAGS_FROM_DIFF(diff) \
	(((diff) << 8) & NWI_IFSTATE_FLAGS_DIFF_MASK)

#define NWI_IFSTATE_DIFF_UNCHANGED	0
#define NWI_IFSTATE_DIFF_ADDED		1
#define NWI_IFSTATE_DIFF_REMOVED	2
#define NWI_IFSTATE_DIFF_CHANGED	3
#define NWI_IFSTATE_DIFF_RANK_UP	4
#define NWI_IFSTATE_DIFF_RANK_DOWN	5


static void
nwi_state_fix_af_aliases(nwi_state_t state, uint32_t old_max_if_count)
{
	int64_t		offset;
	int 		i;
	nwi_ifstate_t	ifstate;

	offset = state->max_if_count - old_max_if_count;
	if (offset < 0) {
		syslog(LOG_ERR, "new count %d < old count %d",
		       state->max_if_count, old_max_if_count);
		return;
	}

	/* iterate IPv4 list and add the offset to any entries with an alias */
	for (i = 0, ifstate = nwi_state_ifstate_list(state, AF_INET);
	     i < state->ipv4_count;
	     i++, ifstate++) {
		if (ifstate->af_alias_offset != 0) {
			/* IPv6 is higher in memory, alias is forward */
			ifstate->af_alias_offset += offset;
		}
	}

	/* iterate IPv6 list and add the offset to any entries with an alias */
	for (i = 0, ifstate = nwi_state_ifstate_list(state, AF_INET6);
	     i < state->ipv6_count;
	     i++, ifstate++) {
		if (ifstate->af_alias_offset != 0) {
			/* IPv6 is higher in memory, alias is backward */
			ifstate->af_alias_offset -= offset;
		}
	}
	return;
}

static void
nwi_state_add_to_if_list(nwi_state_t state, nwi_ifstate_t ifstate)
{
	int		i;
	nwi_ifindex_t *	scan;

	if ((ifstate->flags & NWI_IFSTATE_FLAGS_NOT_IN_IFLIST) != 0) {
		/* doesn't get added to interface list */
		return;
	}
	if (state->if_list_count >= state->max_if_count) {
		/* sanity check */
		return;
	}
	for (i = 0, scan = nwi_state_if_list(state);
	     i < state->if_list_count;
	     i++, scan++) {
		nwi_ifstate_t	this_ifstate;

		this_ifstate = state->ifstate_list + *scan;
		if (strcmp(this_ifstate->ifname, ifstate->ifname) == 0) {
			/* it's already in the list */
			return;
		}
	}
	/* add it to the end */
	*scan = (nwi_ifindex_t)(ifstate - state->ifstate_list);
	state->if_list_count++;
	return;
}

static void
nwi_state_set_if_list(nwi_state_t state)
{
	nwi_ifstate_t	scan_v4;
	nwi_ifstate_t	scan_v6;
	int		v4;
	int		v6;

	v4 = 0;
	v6 = 0;
	state->if_list_count = 0;
	scan_v4 = nwi_state_get_ifstate_with_index(state, AF_INET, v4);
	scan_v6 = nwi_state_get_ifstate_with_index(state, AF_INET6, v6);
	while (scan_v4 != NULL || scan_v6 != NULL) {
		boolean_t	add_v4 = FALSE;

		if (scan_v4 != NULL && scan_v6 != NULL) {
			/* add the higher rank of v4 or v6 */
			if (scan_v4->rank <= scan_v6->rank) {
				add_v4 = TRUE;
			}
		}
		else if (scan_v4 != NULL) {
			add_v4 = TRUE;
		}
		if (add_v4) {
			/* add v4 interface */
			nwi_state_add_to_if_list(state, scan_v4);
			v4++;
			scan_v4 = nwi_state_get_ifstate_with_index(state,
								   AF_INET,
								   v4);
		}
		else {
			/* add v6 interface, move to next item */
			nwi_state_add_to_if_list(state, scan_v6);
			v6++;
			scan_v6 = nwi_state_get_ifstate_with_index(state,
								   AF_INET6,
								   v6);
		}
	}
	return;
}

__private_extern__
nwi_state_t
nwi_state_make_copy(nwi_state_t src)
{
	nwi_state_t	dest = NULL;
	size_t		size;

	if (src == NULL) {
		return dest;
	}
	size = nwi_state_size(src);
	dest = (nwi_state_t)malloc(size);

	if (dest != NULL) {
		memcpy(dest, src, size);
	}
	return dest;
}

__private_extern__
nwi_state_t
nwi_state_new(nwi_state_t old_state, int max_if_count)
{
	size_t	 	size;
	nwi_state_t 	state = NULL;

	if (old_state == NULL && max_if_count == 0) {
		return NULL;
	}

	/* Should we reallocate? */
	if (old_state != NULL) {
		if (old_state->max_if_count >= max_if_count) {
			/* if we're staying the same or shrinking, don't grow */
			return (old_state);
		}
	}
	size = nwi_state_compute_size(max_if_count);
	state = (nwi_state_t)malloc(size);
	memset(state, 0, size);
	state->max_if_count = max_if_count;
	state->version = NWI_STATE_VERSION;

	if (old_state != NULL) {
		state->ipv6_count = old_state->ipv6_count;
		if (state->ipv6_count > 0) {
			memcpy((void *)&state->ifstate_list[state->max_if_count],
			       (void *)&old_state->ifstate_list[old_state->max_if_count],
			       old_state->ipv6_count * sizeof(nwi_ifstate));
		}

		state->ipv4_count = old_state->ipv4_count;
		if (state->ipv4_count > 0) {
			memcpy((void *)state->ifstate_list,
			       (void *)old_state->ifstate_list,
			       old_state->ipv4_count * sizeof(nwi_ifstate));
		}
		/* we grew the arrays so re-compute the offsets */
		nwi_state_fix_af_aliases(state, old_state->max_if_count);
		nwi_state_set_if_list(state);
		nwi_state_free(old_state);
	} else {
		state->ipv4_count = 0;
		state->ipv6_count = 0;
	}
	return state;
}

__private_extern__ void
nwi_state_finalize(nwi_state_t state)
{
	if (state == NULL) {
		return;
	}
	nwi_state_set_if_list(state);
	return;
}

static __inline__
nwi_ifstate_t
nwi_state_get_last_ifstate(nwi_state_t state, int af, nwi_ifindex_t** last)
{
	nwi_ifindex_t *	count;
	int		idx;

	assert(state != NULL);

	count = (af == AF_INET) ? &state->ipv4_count
				: &state->ipv6_count;

	idx = (af == AF_INET) ? state->ipv4_count
			      : (state->max_if_count + state->ipv6_count);

	*last = count;

	return &state->ifstate_list[idx];
}

__private_extern__
void
nwi_ifstate_set_signature(nwi_ifstate_t ifstate, uint8_t * signature)
{
	memcpy(ifstate->signature, signature, sizeof(ifstate->signature));
	ifstate->flags |= NWI_IFSTATE_FLAGS_HAS_SIGNATURE;
	return;
}

static void
nwi_state_add_ifstate_alias(nwi_state_t state, nwi_ifstate_t ifstate)
{
	nwi_ifstate_t	alias;

	alias = nwi_state_get_ifstate_with_name(state,
						nwi_other_af(ifstate->af),
						ifstate->ifname);
	if (alias == NULL) {
		return;
	}
	ifstate->af_alias_offset = (nwi_ifindex_t)(alias - ifstate);
	alias->af_alias_offset = (nwi_ifindex_t)(ifstate - alias);
	return;
}

__private_extern__ nwi_ifstate_t
nwi_state_add_ifstate(nwi_state_t state,
		      const char * ifname, int af,
		      uint64_t flags, Rank rank,
		      void * ifa,
		      struct sockaddr * vpn_server_addr,
		      uint32_t reach_flags)
{
	nwi_ifstate_t 	ifstate;

	/* Will only add unique elements to the list */
	ifstate = nwi_state_get_ifstate_with_name(state, af, ifname);

	/* Already present, just ignore it */
	if (ifstate != NULL) {
		if (ifstate->rank < rank) {
			/* always true because they are added in order */
			return NULL;
		}
	}
	else {
		/* add to the end */
		nwi_ifindex_t	count;
		nwi_ifindex_t *	count_p;

		/* make sure we aren't already full */
		ifstate = nwi_state_get_last_ifstate(state, af, &count_p);
		count = *count_p;
		if (count == state->max_if_count) {
			/* should not happen */
			syslog(LOG_ERR,
			       "nwi_state_add_ifstate: full at count %d\n",
			       count);
			return (NULL);
		}
		if (count > 0) {
			/* previous ifstate is no longer last */
			nwi_ifstate_t	prev = ifstate - 1;

			prev->flags &= ~NWI_IFSTATE_FLAGS_LAST_ITEM;
		}
		memset(ifstate, 0, sizeof(*ifstate));
		strlcpy(ifstate->ifname, ifname, sizeof(ifstate->ifname));
		ifstate->af = af;
		/* this is the new last ifstate */
		ifstate->flags |= NWI_IFSTATE_FLAGS_LAST_ITEM;
		(*count_p)++;

		/* add the alias */
		nwi_state_add_ifstate_alias(state, ifstate);
	}

	/* We need to update the address/rank/flag fields for the existing/new
	 * element */
	if (ifa != NULL) {
		switch (af) {
			case AF_INET:
				ifstate->iaddr = *((struct in_addr *) ifa);
				break;
			case AF_INET6:
				ifstate->iaddr6 = *((struct in6_addr *) ifa);
				break;
			default:
				break;
		}

	}

	if (vpn_server_addr != NULL && vpn_server_addr->sa_family != 0) {
		_nwi_ifstate_set_vpn_server(ifstate, vpn_server_addr);
	} else {
		_nwi_ifstate_set_vpn_server(ifstate, NULL);
	}

	ifstate->reach_flags = reach_flags;
	ifstate->rank = rank;
	ifstate->flags &= ~NWI_IFSTATE_FLAGS_MASK;
	ifstate->flags |= NWI_IFSTATE_FLAGS(flags);
	return ifstate;
}

__private_extern__
void
nwi_state_clear(nwi_state_t state, int af)
{
	if (af == AF_INET) {
		state->ipv4_count = 0;
	}
	else {
		state->ipv6_count = 0;
	}
	return;

}

__private_extern__
void *
nwi_ifstate_get_address(nwi_ifstate_t ifstate)
{
	return (void *)&ifstate->iaddr;
}


static __inline__ uint8_t
nwi_ifstate_get_diff(nwi_ifstate_t ifstate)
{
	return (NWI_IFSTATE_FLAGS_GET_DIFF(ifstate->flags));
}

__private_extern__ const char *
nwi_ifstate_get_diff_str(nwi_ifstate_t ifstate)
{
	const char *	strings[] = {
		"",
		"+",
		"-",
		"!",
		"/",
		"\\"
	};
	uint8_t		diff;

	diff = nwi_ifstate_get_diff(ifstate);
	if (diff < sizeof(strings) / sizeof(strings[0])) {
		return (strings[diff]);
	}
	return ("?");
}

__private_extern__ nwi_ifstate_difference_t
nwi_ifstate_get_difference(nwi_ifstate_t diff_ifstate)
{
	nwi_ifstate_difference_t	diff;

	switch (nwi_ifstate_get_diff(diff_ifstate)) {
	case NWI_IFSTATE_DIFF_ADDED:
	case NWI_IFSTATE_DIFF_CHANGED:
		diff = knwi_ifstate_difference_changed;
		break;
	case NWI_IFSTATE_DIFF_REMOVED:
		diff = knwi_ifstate_difference_removed;
		break;
	default:
		diff = knwi_ifstate_difference_none;
		break;
	}
	return (diff);
}

static inline boolean_t
nwi_ifstate_has_changed(nwi_ifstate_t ifstate1, nwi_ifstate_t ifstate2)
{
	if (NWI_IFSTATE_FLAGS(ifstate1->flags)
	    != NWI_IFSTATE_FLAGS(ifstate2->flags)) {
		return TRUE;
	}

	if (ifstate1->af == AF_INET) {
		if (memcmp(&ifstate1->iaddr, &ifstate2->iaddr, sizeof(struct in_addr)) != 0) {
			return TRUE;
		}
	} else {
		if (memcmp(&ifstate1->iaddr6, &ifstate2->iaddr6, sizeof(struct in6_addr)) != 0) {
			return TRUE;
		}
	}
	return FALSE;
}

static inline nwi_ifstate_t
nwi_state_diff_append(nwi_state_t state, nwi_ifstate_t scan)
{
	nwi_ifindex_t *	last;
	nwi_ifstate_t 	new_ifstate = NULL;

	new_ifstate = nwi_state_get_last_ifstate(state, scan->af, &last);
	memcpy(new_ifstate, scan, sizeof(*scan));
	(*last)++;
	return new_ifstate;
}

static inline void
nwi_ifstate_set_diff(nwi_ifstate_t ifstate, uint8_t diff)
{
	ifstate->flags &= ~NWI_IFSTATE_FLAGS_DIFF_MASK;
	if (diff != NWI_IFSTATE_DIFF_UNCHANGED) {
		ifstate->flags |= NWI_IFSTATE_FLAGS_FROM_DIFF(diff);
	}
}

static void
nwi_state_diff_add_change(nwi_state_t diff, nwi_state_t old_ifstate,
			  nwi_ifstate_t ifstate)
{
	nwi_ifstate_t 	existing;
	nwi_ifstate_t 	new_ifstate;

	existing = nwi_state_get_ifstate_with_name(old_ifstate,
						   ifstate->af,
						   nwi_ifstate_get_ifname(ifstate));
	new_ifstate = nwi_state_diff_append(diff, ifstate);
	if (existing != NULL) {
		if (nwi_ifstate_has_changed(existing, new_ifstate)) {
			nwi_ifstate_set_diff(new_ifstate,
					     NWI_IFSTATE_DIFF_CHANGED);
		} else if (existing->rank < new_ifstate->rank) {
			nwi_ifstate_set_diff(new_ifstate,
					     NWI_IFSTATE_DIFF_RANK_DOWN);
		} else if (existing->rank > new_ifstate->rank) {
			nwi_ifstate_set_diff(new_ifstate,
					     NWI_IFSTATE_DIFF_RANK_UP);
		} else {
			nwi_ifstate_set_diff(new_ifstate,
					     NWI_IFSTATE_DIFF_UNCHANGED);
		}
	} else {
		nwi_ifstate_set_diff(new_ifstate, NWI_IFSTATE_DIFF_ADDED);
	}
	return;
}

static void
nwi_state_diff_remove(nwi_state_t state, nwi_ifstate_t ifstate)
{
	nwi_ifstate_t 	removed_ifstate;

	if (nwi_state_get_ifstate_with_name(state,
					    ifstate->af,
					    nwi_ifstate_get_ifname(ifstate))
	    != NULL) {
		/* there's still an ifstate */
		return;
	}
	removed_ifstate = nwi_state_diff_append(state, ifstate);
	nwi_ifstate_set_diff(removed_ifstate, NWI_IFSTATE_DIFF_REMOVED);
	return;
}

static void
nwi_state_diff_populate(nwi_state_t diff, nwi_state_t old_ifstate, nwi_state_t new_ifstate)
{
	int i;
	nwi_ifstate_t scan;

	if (new_ifstate != NULL) {
		/* check for adds/changes */
		if (new_ifstate->ipv4_count) {
			for (i = 0, scan = new_ifstate->ifstate_list;
			     i < new_ifstate->ipv4_count; i++, scan++) {
				nwi_state_diff_add_change(diff, old_ifstate, scan);
			}
		}
		if (new_ifstate->ipv6_count) {
			scan = new_ifstate->ifstate_list + new_ifstate->max_if_count;
			for (i = 0;
			     i < new_ifstate->ipv6_count; i++, scan++) {
				nwi_state_diff_add_change(diff, old_ifstate, scan);
			}
		}
	}
	if (old_ifstate != NULL) {
		/* check for removes */
		if (old_ifstate->ipv4_count) {
			for (i = 0, scan = old_ifstate->ifstate_list;
			     i < old_ifstate->ipv4_count; i++, scan++) {
				nwi_state_diff_remove(diff, scan);
			}
		}
		if (old_ifstate->ipv6_count) {
			scan = old_ifstate->ifstate_list + old_ifstate->max_if_count;
			for (i = 0;
			     i < old_ifstate->ipv6_count; i++, scan++) {
				nwi_state_diff_remove(diff, scan);
			}
		}
	}
	return;
}

static int
nwi_state_max_af_count(nwi_state_t state)
{
	if (state->ipv4_count >= state->ipv6_count) {
		return (state->ipv4_count);
	}
	return (state->ipv6_count);
}

__private_extern__ nwi_state_t
nwi_state_diff(nwi_state_t old_ifstate, nwi_state_t new_ifstate)
{
	nwi_state_t	diff;
	int		total_count = 0;

	/*
	 * Compute the worst case total number of elements we need:
	 * 	the max count of (IPv4, IPv6) in the old
	 *    + the max count of (IPv4, IPv6) in the new
	 * Worst case assumes that the old and new share none of the
	 * same interfaces.
	 */
	if (old_ifstate != NULL) {
		total_count += nwi_state_max_af_count(old_ifstate);
	}
	if (new_ifstate != NULL) {
		total_count += nwi_state_max_af_count(new_ifstate);
	}
	if (total_count == 0) {
		return NULL;
	}

	diff = nwi_state_new(NULL, total_count);
	nwi_state_diff_populate(diff, old_ifstate, new_ifstate);

	/* diff consists of a nwi_state_t with diff flags on each ifstate */
	return diff;
}

static __inline__
void
_nwi_ifstate_set_generation(nwi_ifstate_t ifstate, uint64_t generation_count)
{
	ifstate->if_generation_count = generation_count;

	return;
}

static
boolean_t
_nwi_ifstate_has_changed(nwi_state_t state, const char * ifname)
{
	nwi_ifstate_t 	ifstate;

	/* If either the v4 ifstate or the v6 ifstate
	 * has changed, then report that the interface has changed */
	ifstate = nwi_state_get_ifstate_with_name(state,
						  AF_INET,
						  ifname);

	if (ifstate != NULL
	    && nwi_ifstate_get_diff(ifstate) != NWI_IFSTATE_DIFF_UNCHANGED) {
		return (TRUE);
	}

	ifstate = nwi_state_get_ifstate_with_name(state,
						  AF_INET6,
						  ifname);

	if (ifstate != NULL
	    && nwi_ifstate_get_diff(ifstate) != NWI_IFSTATE_DIFF_UNCHANGED) {
		return (TRUE);
	}
	return (FALSE);
}

__private_extern__
void
_nwi_state_update_interface_generations(nwi_state_t old_state, nwi_state_t state, nwi_state_t changes)
{
	int		i;
	uint64_t	generation_count;
	nwi_ifstate_t	scan;

	if (state == NULL || changes == NULL) {
		return;
	}

	/* cache the generation count */
	generation_count = state->generation_count;

	for (i = 0, scan = nwi_state_ifstate_list(state, AF_INET);
	     i < state->ipv4_count; i++, scan++) {
		if (_nwi_ifstate_has_changed(changes, scan->ifname)) {
			/* Update the interface generation count */
			_nwi_ifstate_set_generation(scan, generation_count);
		} else {
			nwi_ifstate_t old_ifstate;

			old_ifstate = nwi_state_get_ifstate_with_name(old_state,
								      AF_INET,
								      scan->ifname);
			assert(old_ifstate != NULL);

			/* Set the current generation count */
			_nwi_ifstate_set_generation(scan,
						    old_ifstate->if_generation_count);
		}
	}
	for (i = 0, scan = nwi_state_ifstate_list(state, AF_INET6);
	     i < state->ipv6_count; i++, scan++) {
		/* The generation count has been already updated in
		 * the ipv4 case, just skip it. */
		if (nwi_ifstate_get_generation(scan) ==
		    generation_count) {
			continue;
		}
		if (_nwi_ifstate_has_changed(changes, scan->ifname)) {
			/* update the interface generation count */
			_nwi_ifstate_set_generation(scan, generation_count);
		} else {
			nwi_ifstate_t old_ifstate;

			old_ifstate = nwi_state_get_ifstate_with_name(old_state,
								      AF_INET6,
								      scan->ifname);
			assert(old_ifstate != NULL);

			/* Set the current generation count */
			_nwi_ifstate_set_generation(scan,
						    old_ifstate->if_generation_count);
		}
	}
	return;
}

__private_extern__
void
_nwi_state_compute_sha256_hash(nwi_state_t state,
			       unsigned char hash[CC_SHA256_DIGEST_LENGTH])
{
	CC_SHA256_CTX	ctx;
	uint64_t	generation_save;

	if (state == NULL) {
		memset(hash, 0, CC_SHA256_DIGEST_LENGTH);
		return;
	}

	/* preserve generation */
	generation_save = state->generation_count;

	/* zero out the generation count before computing hash */
	state->generation_count = 0;

	/* compute hash */
	CC_SHA256_Init(&ctx);
	CC_SHA256_Update(&ctx, state, (CC_LONG)nwi_state_size(state));
	CC_SHA256_Final(hash, &ctx);

	/* restore generation */
	state->generation_count = generation_save;

	return;
}
