/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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

#ifndef _NET_IF_BOND_VAR_H_
#define _NET_IF_BOND_VAR_H_

#include <sys/types.h>

#include <net/lacp.h>

#pragma pack(4)

#define IF_BOND_OP_ADD_INTERFACE                1
#define IF_BOND_OP_REMOVE_INTERFACE             2
#define IF_BOND_OP_GET_STATUS                   3
#define IF_BOND_OP_SET_VERBOSE                  4
#define IF_BOND_OP_SET_MODE                     5

#define IF_BOND_MODE_LACP                       0
#define IF_BOND_MODE_STATIC                     1

struct if_bond_partner_state {
	lacp_system                 ibps_system;
	lacp_system_priority        ibps_system_priority;
	lacp_key                    ibps_key;
	lacp_port                   ibps_port;
	lacp_port_priority          ibps_port_priority;
	lacp_actor_partner_state    ibps_state;
	u_char                      ibps_reserved1;
};

#define IF_BOND_STATUS_SELECTED_STATE_UNSELECTED        0
#define IF_BOND_STATUS_SELECTED_STATE_SELECTED          1
#define IF_BOND_STATUS_SELECTED_STATE_STANDBY           2

struct if_bond_status {
	char                        ibs_if_name[IFNAMSIZ];/* interface name */
	lacp_port_priority          ibs_port_priority;
	lacp_actor_partner_state    ibs_state;
	u_char                      ibs_selected_state;
	struct if_bond_partner_state ibs_partner_state;
	u_int32_t                   ibs_reserved[8];
};

#define IF_BOND_STATUS_REQ_VERSION      1

struct if_bond_status_req {
	int         ibsr_version;/* version */
	int         ibsr_total; /* returned number of struct if_bond_status's */
	int         ibsr_count; /* number that will fit in ibsr_buffer */
	union {                 /* buffer to hold if_bond_status's */
		void *          ibsru_buffer;
		u_int64_t       ibsru_buffer64;
	} ibsr_ibsru;
	lacp_key    ibsr_key;   /* returned */
	u_int8_t    ibsr_mode;  /* returned (IF_BOND_MODE_{LACP, STATIC}) */
	u_int8_t    ibsr_reserved0;/* for future use */
	u_int32_t   ibsr_reserved[3];/* for future use */
};
#define ibsr_buffer     ibsr_ibsru.ibsru_buffer

struct if_bond_req {
	u_int32_t   ibr_op;                     /* operation */
	union {
		char    ibru_if_name[IFNAMSIZ]; /* interface name */
		struct if_bond_status_req ibru_status; /* status information */
		int     ibru_int_val;
	} ibr_ibru;
};

#pragma pack()

#include <net/if_bond_internal.h>

#endif /* _NET_IF_BOND_VAR_H_ */
