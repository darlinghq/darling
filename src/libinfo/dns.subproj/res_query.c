/*
 * Copyright (c) 1999, 2012 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <dns_sd.h>
#include <errno.h>

#include <arpa/nameser_compat.h>
#include <nameser.h>

#include "si_module.h"

extern int h_errno;

// Storage for the global struct __res_9_state.
// The BIND9 libresolv.dylib shares the same storage for this structure as the
// legacy BIND8 libsystem_info.dylib. This implementation does not require the
// _res structure but libresolv.dylib does and many 3rd-party applications
// access this global symbol directly so we preserve it here.
#ifdef __LP64__
#define RES_9_STATE_SIZE 552
#else
#define RES_9_STATE_SIZE 512
#endif
char _res[RES_9_STATE_SIZE] = {0};

int
res_init(void)
{
	// For compatibility only.
	return 0;
}

// Perform a DNS query. Returned DNS response is placed in the answer buffer.
// A preliminary check of the answer is performed and success is returned only
// if no error is indicated in the answer and the answer count is nonzero.
// Returns the size of the response on success, or -1 with h_errno set.
static int
_mdns_query(int call, const char *name, int class, int type, u_char *answer, int anslen)
{
	int res = -1;
	si_item_t *item;
	uint32_t err;
	
	si_mod_t *dns = si_module_with_name("mdns");
	if (dns == NULL) {
		h_errno = NO_RECOVERY;
		return -1;
	}
	
	item = dns->vtable->sim_item_call(dns, call, name, NULL, NULL, class, type, &err);
	
	if (item != NULL) {
		si_dnspacket_t *p;
		
		p = (si_dnspacket_t *)((uintptr_t)item + sizeof(si_item_t));
		
		res = p->dns_packet_len;
		
		// Truncate to destination buffer size.
		memcpy(answer, p->dns_packet, MIN(res, anslen));
		
		si_item_release(item);
	} else {
		h_errno = HOST_NOT_FOUND;
		res = -1;
	}

	if (MIN(res, anslen) >= sizeof(HEADER)) {
		HEADER *hp = (HEADER *)answer;
		switch (hp->rcode) {
			case NXDOMAIN:
				h_errno = HOST_NOT_FOUND;
				res = -1;
				break;
			case SERVFAIL:
				h_errno = TRY_AGAIN;
				res = -1;
				break;
			case NOERROR:
				if (ntohs(hp->ancount) == 0) {
					h_errno = NO_DATA;
					res = -1;
				}
				break;
			case FORMERR:
			case NOTIMP:
			case REFUSED:
			default:
				h_errno = NO_RECOVERY;
				res = -1;
				break;
		}
	}

	si_module_release(dns);
	return res;
}

int
res_query(const char *name, int class, int type, u_char *answer, int anslen)
{
	return _mdns_query(SI_CALL_DNS_QUERY, name, class, type, answer, anslen);
}

int
res_search(const char *name, int class, int type, u_char *answer, int anslen)
{
	return _mdns_query(SI_CALL_DNS_SEARCH, name, class, type, answer, anslen);
}
