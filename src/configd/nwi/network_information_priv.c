/*
 * Copyright (c) 2011-2013 Apple Inc. All rights reserved.
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
#include "network_information_priv.h"
#include <limits.h>
#include <stdio.h>

__private_extern__
const sa_family_t nwi_af_list[] = {AF_INET, AF_INET6};

static __inline__ size_t
nwi_state_compute_size(unsigned int n)
{
	return (offsetof(nwi_state, nwi_ifstates[n]));

}
__private_extern__
nwi_state_t
nwi_state_copy_priv(nwi_state_t src)
{
	nwi_state_t dest = NULL;

	if (src == NULL) {
		return dest;
	}

	dest = malloc(src->size);

	if (dest != NULL) {
		bcopy(src, dest, src->size);

		dest->ref = 1;
		dest->svr = FALSE;
	}
	return dest;
}

__private_extern__
nwi_state_t
nwi_state_new(nwi_state_t old_state, int elems)
{
	nwi_state_t state = NULL;
	uint32_t new_size;

	if (old_state == NULL && elems == 0) {
		return NULL;
	}

	/* Need to insert a last node for each of the v4/v6 list */
	new_size = (elems != 0)
			? (uint32_t)(sizeof(nwi_state) + nwi_state_compute_size((elems+1) * 2))
			: 0;

	/* Should we reallocate? */
	if (old_state != NULL) {
		if (old_state->size >= new_size) {
			return (old_state);
		}
	}

	state = malloc(new_size);
	bzero(state, new_size);
	state->size = new_size;

	/*
	 * v4 list is stored 0 to elems,
	 * v6 list is stored elems + 1 to 2 * elems + 2
	 */
	state->ipv6_start = elems + 1;

	if (old_state != NULL) {
		state->ipv6_count = old_state->ipv6_count;
		if (state->ipv6_count > 0) {
			bcopy((void*) &old_state->nwi_ifstates[old_state->ipv6_start],
			      (void*) &state->nwi_ifstates[state->ipv6_start],
			      old_state->ipv6_count * sizeof(nwi_ifstate));
		}

		state->ipv4_count = old_state->ipv4_count;
		if (state->ipv4_count > 0) {
			bcopy((void*) old_state->nwi_ifstates,
			      (void*) state->nwi_ifstates,
			      old_state->ipv4_count * sizeof(nwi_ifstate));
		}

		free(old_state);
	} else {
		state->ipv4_count = 0;
		state->ipv6_count = 0;
	}
	nwi_state_set_last(state, AF_INET);
	nwi_state_set_last(state, AF_INET6);

	state->ref = 1;
	state->svr = FALSE;
	return state;
}

static __inline__
nwi_ifstate_t
nwi_ifstate_get_last(nwi_state_t state, int af, uint32_t** last)
{
	uint32_t*	count;
	int		idx;

	assert(state != NULL);

	count = (af == AF_INET) ? &state->ipv4_count
				: &state->ipv6_count;

	idx = (af == AF_INET) ? state->ipv4_count
			      : (state->ipv6_start + state->ipv6_count);

	*last = count;

	return &state->nwi_ifstates[idx];
}

__private_extern__
void
nwi_ifstate_set_signature(nwi_ifstate_t ifstate, uint8_t * signature)
{
	bcopy(signature, ifstate->signature, sizeof(ifstate->signature));
	ifstate->flags |= NWI_IFSTATE_FLAGS_HAS_SIGNATURE;
	return;
}

__private_extern__
nwi_ifstate_t
nwi_insert_ifstate(nwi_state_t state,
		   const char * ifname, int af,
		   uint64_t flags, Rank rank,
		   void * ifa,
		   struct sockaddr * vpn_server_addr,
		   uint32_t reach_flags)
{
	nwi_ifstate_t 	ifstate;

	/* Will only insert unique elements in the list */
	ifstate = nwi_state_get_ifstate_with_name(state, af, ifname);

	/* Already present, just ignore it */
	if (ifstate != NULL) {
		if (ifstate->rank < rank) {
			return NULL;
		}
	}

	if (ifstate == NULL) {
		uint32_t	*last;

		/* We need to append it as the last element */
		ifstate = nwi_ifstate_get_last(state, af, &last);
		bzero(ifstate, sizeof(*ifstate));
		strlcpy(ifstate->ifname, ifname, sizeof(ifstate->ifname));
		ifstate->af = af;
		(*last)++;
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
	ifstate->flags = flags;

	return ifstate;
}

__private_extern__
void
nwi_state_clear(nwi_state_t state, int af)
{
	uint32_t* count;

	count = (af == AF_INET)
		?&state->ipv4_count:&state->ipv6_count;

	*count = 0;
	nwi_state_set_last(state, af);
	return;

}

__private_extern__
void
nwi_state_set_last(nwi_state_t state, int af)
{
	int last_elem_idx;

	if (state == NULL) {
		return;
	}

	/* The last element is an element with the flags set as
	 * NWI_IFSTATE_FLAGS_NOT_IN_LIST */
	last_elem_idx = (af == AF_INET) ? state->ipv4_count
					: (state->ipv6_start + state->ipv6_count);

	state->nwi_ifstates[last_elem_idx].ifname[0] = '\0';
	state->nwi_ifstates[last_elem_idx].flags = NWI_IFSTATE_FLAGS_NOT_IN_LIST;
}

#define	unchanged	""
#define added		"+"
#define deleted		"-"
#define changed		"!"
#define rank_change	"R"

__private_extern__
void *
nwi_ifstate_get_address(nwi_ifstate_t ifstate)
{
	return (void *)&ifstate->iaddr;
}

__private_extern__
const char *
nwi_ifstate_get_diff_str(nwi_ifstate_t ifstate)
{
	if (strcmp(ifstate->diff_str, rank_change) == 0) {
		return changed;
	}
	return ifstate->diff_str;
}

static
inline
boolean_t
nwi_ifstate_has_changed(nwi_ifstate_t ifstate1, nwi_ifstate_t ifstate2)
{
	if (ifstate1->flags != ifstate2->flags) {
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

static
inline
nwi_ifstate_t
nwi_ifstate_append(nwi_state_t state, nwi_ifstate_t scan)
{
	nwi_ifstate_t 	new_ifstate = NULL;
	uint32_t	*last;

	new_ifstate = nwi_ifstate_get_last(state, scan->af, &last);
	memcpy(new_ifstate, scan, sizeof(*scan));
	(*last)++;
	return new_ifstate;
}

static
inline
void
nwi_ifstate_set_diff_str(nwi_ifstate_t ifstate, const char *diff_str)
{
	ifstate->diff_str = diff_str;
}

static
void
nwi_ifstate_set_added_or_changed_str(nwi_state_t state, nwi_state_t old_state, nwi_ifstate_t ifstate)
{
	nwi_ifstate_t 	existing_ifstate, new_ifstate;

	existing_ifstate = nwi_state_get_ifstate_with_name(old_state,
							   ifstate->af,
							   nwi_ifstate_get_ifname(ifstate));

	/* Add the element that is not in the store */
	new_ifstate = nwi_ifstate_append(state, ifstate);

	/* These are potentially "added" elements unless they are
	 * in the old list */
	nwi_ifstate_set_diff_str(new_ifstate, added);

	if (existing_ifstate != NULL) {
		if (nwi_ifstate_has_changed(existing_ifstate, new_ifstate) == TRUE) {
			nwi_ifstate_set_diff_str(new_ifstate, changed);
		} else if (existing_ifstate->rank != new_ifstate->rank) {
			nwi_ifstate_set_diff_str(new_ifstate, rank_change);
		} else {
			nwi_ifstate_set_diff_str(new_ifstate, unchanged);
		}
	}
	return;
}

static
void
nwi_ifstate_set_removed_str(nwi_state_t state, nwi_ifstate_t ifstate)
{
	nwi_ifstate_t 	existing_ifstate;

	existing_ifstate = nwi_state_get_ifstate_with_name(state,
							   ifstate->af,
							   nwi_ifstate_get_ifname(ifstate));

	/* Any elements that has not been added means that they are removed */
	if (existing_ifstate == NULL) {
		nwi_ifstate_t new_ifstate = nwi_ifstate_append(state, ifstate);
		nwi_ifstate_set_diff_str(new_ifstate, deleted);
	}
	return;
}

static
void
nwi_state_merge_added(nwi_state_t state, nwi_state_t old_state,
		      nwi_state_t new_state)
{
	int i;
	nwi_ifstate_t scan;

	if (new_state == NULL) {
		return;
	}

	if (new_state->ipv4_count) {
		for (i = 0, scan = new_state->nwi_ifstates;
		     i < new_state->ipv4_count; i++, scan++) {
			nwi_ifstate_set_added_or_changed_str(state, old_state, scan);
		}
		nwi_state_set_last(state, AF_INET);
	}

	if (new_state->ipv6_count) {
		for (i = 0, scan = new_state->nwi_ifstates + new_state->ipv6_start;
		     i < new_state->ipv6_count; i++, scan++) {
			nwi_ifstate_set_added_or_changed_str(state, old_state, scan);
		}
		nwi_state_set_last(state, AF_INET6);
	}
	return;
}

void
nwi_state_merge_removed(nwi_state_t state, nwi_state_t old_state)
{
	int i;
	nwi_ifstate_t scan;

	if (old_state == NULL) {
		return;
	}

	if (old_state->ipv4_count) {
		for (i = 0, scan = old_state->nwi_ifstates;
		     i < old_state->ipv4_count; i++, scan++) {
			nwi_ifstate_set_removed_str(state, scan);
		}
		nwi_state_set_last(state, AF_INET);
	}

	if (old_state->ipv6_count) {
		for (i = 0, scan = old_state->nwi_ifstates + old_state->ipv6_start;
		     i < old_state->ipv6_count; i++, scan++) {
			nwi_ifstate_set_removed_str(state, scan);
		}
		nwi_state_set_last(state, AF_INET6);
	}
	return;
}

__private_extern__
nwi_state_t
nwi_state_diff(nwi_state_t old_state, nwi_state_t new_state)
{
	nwi_state_t	diff;
	int		total_count = 0;

	if (old_state != NULL) {
		total_count = old_state->ipv4_count + old_state->ipv6_count;
	}

	if (new_state != NULL) {
		total_count += new_state->ipv4_count + new_state->ipv6_count;
	}

	if (total_count == 0) {
		return NULL;
	}

	diff = nwi_state_new(NULL, total_count);

	nwi_state_merge_added(diff, old_state, new_state);
	nwi_state_merge_removed(diff, old_state);

	/* Diff consists of a nwi_state_t with annotated diff_str's */
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
	    && ifstate->diff_str != NULL
	    && strcmp(ifstate->diff_str, unchanged) != 0
	    && strcmp(ifstate->diff_str, rank_change) != 0) {
		return (TRUE);
	}

	ifstate = nwi_state_get_ifstate_with_name(state,
						  AF_INET6,
						  ifname);

	if (ifstate != NULL
	    && ifstate->diff_str != NULL
	    && strcmp(ifstate->diff_str, unchanged) != 0
	    && strcmp(ifstate->diff_str, rank_change) != 0) {
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

	if (state->ipv4_count) {
		for (i = 0, scan = state->nwi_ifstates;
		     i < state->ipv4_count; i++, scan++) {
			if (_nwi_ifstate_has_changed(changes, scan->ifname) == TRUE) {
				/* Update the interface generation count */
				_nwi_ifstate_set_generation(scan, generation_count);
			} else {
				nwi_ifstate_t old_ifstate;

				old_ifstate = nwi_state_get_ifstate_with_name(old_state,
									      AF_INET,
									      scan->ifname);

				/* Set the current generation count */
				_nwi_ifstate_set_generation(scan,
							    old_ifstate->if_generation_count);
			}
		}
	}

	if (state->ipv6_count) {
		for (i = 0, scan = state->nwi_ifstates + state->ipv6_start;
		     i < state->ipv6_count; i++, scan++) {
			/* The generation count has been already updated in
			 * the ipv4 case, just skip it. */
			if (nwi_ifstate_get_generation(scan) ==
				generation_count) {
				continue;
			}
			if (_nwi_ifstate_has_changed(changes, scan->ifname) == TRUE) {
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
	}
	return;
}
