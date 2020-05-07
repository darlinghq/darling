/*
 * Copyright (c) 2012-2017 Apple Inc. All rights reserved.
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

#ifndef __FLOW_DIVERT_H__
#define __FLOW_DIVERT_H__

#include <sys/mbuf.h>

struct flow_divert_group;
struct flow_divert_trie_node;

struct flow_divert_pcb {
	decl_lck_mtx_data(, mtx);
	socket_t                                            so;
	RB_ENTRY(flow_divert_pcb)           rb_link;
	uint32_t                                            hash;
	mbuf_t                                                      connect_token;
	struct sockaddr                                     *local_address;
	struct sockaddr                                     *remote_address;
	uint32_t                                            flags;
	uint32_t                                            send_window;
	uint32_t                                            sb_size;
	struct flow_divert_group            *group;
	uint32_t                                            control_group_unit;
	int32_t                                                     ref_count;
	uint32_t                                            bytes_written_by_app;
	uint32_t                                            bytes_read_by_app;
	uint32_t                                            bytes_sent;
	uint32_t                                            bytes_received;
	uint8_t                                                     log_level;
	SLIST_ENTRY(flow_divert_pcb)        tmp_list_entry;
	mbuf_t                                                      connect_packet;
	uint8_t                                                     *app_data;
	size_t                                                      app_data_length;
};

RB_HEAD(fd_pcb_tree, flow_divert_pcb);

struct flow_divert_trie {
	struct flow_divert_trie_node *nodes;
	uint16_t *child_maps;
	uint8_t *bytes;
	void *memory;
	size_t nodes_count;
	size_t child_maps_count;
	size_t bytes_count;
	size_t nodes_free_next;
	size_t child_maps_free_next;
	size_t bytes_free_next;
	uint16_t root;
};

struct flow_divert_group {
	decl_lck_rw_data(, lck);
	struct fd_pcb_tree                          pcb_tree;
	uint32_t                                            ctl_unit;
	uint8_t                                                     atomic_bits;
	MBUFQ_HEAD(send_queue_head)         send_queue;
	uint8_t                                                     *token_key;
	size_t                                                      token_key_size;
	uint32_t                                            flags;
	struct flow_divert_trie                     signing_id_trie;
};

void            flow_divert_init(void);
void            flow_divert_detach(struct socket *so);
errno_t         flow_divert_token_set(struct socket *so, struct sockopt *sopt);
errno_t         flow_divert_token_get(struct socket *so, struct sockopt *sopt);
errno_t         flow_divert_pcb_init(struct socket *so, uint32_t ctl_unit);
errno_t         flow_divert_connect_out(struct socket *so, struct sockaddr *to, proc_t p);
errno_t         flow_divert_implicit_data_out(struct socket *so, int flags, mbuf_t data, struct sockaddr *to, mbuf_t control, struct proc *p);

#endif /* __FLOW_DIVERT_H__ */
