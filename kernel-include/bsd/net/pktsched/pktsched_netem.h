/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#define NETEM_MAX_BATCH_SIZE    32

__BEGIN_DECLS

extern int netem_init(void);
extern void netem_fini(void);

extern int netem_config(struct netem **ne, const char *name,
    const struct if_netem_params *p, void *output_handle,
    int (*output)(void *handle, pktsched_pkt_t *pkts, uint32_t n_pkts),
    uint32_t output_max_batch_size);
extern void netem_get_params(struct netem *ne, struct if_netem_params *p);
extern void netem_destroy(struct netem *ne);
extern int netem_enqueue(struct netem *ne, classq_pkt_t *p, boolean_t *pdrop);
extern int netem_dequeue(struct netem *ne, pktsched_pkt_t *p,
    boolean_t *ppending);

__END_DECLS
