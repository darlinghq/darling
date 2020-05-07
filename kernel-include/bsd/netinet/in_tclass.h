/*
 * Copyright (c) 2015-2017 Apple Inc. All rights reserved.
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

#ifndef __NETINET_IN_TCLASS_H__
#define __NETINET_IN_TCLASS_H__

#ifdef PRIVATE

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/mbuf.h>
#include <net/if.h>
#include <net/if_var.h>

#define SO_TCDBG_PID            0x01    /* Set/get traffic class policy for PID */
#define SO_TCDBG_PNAME          0x02    /* Set/get traffic class policy for processes of that name */
#define SO_TCDBG_PURGE          0x04    /* Purge entries for unused PIDs */
#define SO_TCDBG_FLUSH          0x08    /* Flush all entries */
#define SO_TCDBG_COUNT          0x10    /* Get count of entries */
#define SO_TCDBG_LIST           0x20    /* List entries */
#define SO_TCDBG_DELETE         0x40    /* Delete a process entry */
#define SO_TCDBG_TCFLUSH_PID    0x80    /* Flush traffic class for PID */

struct so_tcdbg {
	u_int32_t       so_tcdbg_cmd;
	int32_t         so_tcdbg_tclass;
	int32_t         so_tcdbg_netsvctype;
	u_int32_t       so_tcdbg_count;
	pid_t           so_tcdbg_pid;
	u_int32_t       so_tcbbg_qos_mode;
	char            so_tcdbg_pname[(2 * MAXCOMLEN) + 1];
};
#define QOS_MODE_MARKING_POLICY_DEFAULT         0
#define QOS_MODE_MARKING_POLICY_ENABLE          1
#define QOS_MODE_MARKING_POLICY_DISABLE         2

#define NET_QOS_MARKING_POLICY_DEFAULT QOS_MODE_MARKING_POLICY_DEFAULT /* obsolete, to be removed */
#define NET_QOS_MARKING_POLICY_ENABLE QOS_MODE_MARKING_POLICY_ENABLE /* obsolete, to be removed */
#define NET_QOS_MARKING_POLICY_DISABLE QOS_MODE_MARKING_POLICY_DISABLE /* obsolete, to be removed */

struct net_qos_param {
	u_int64_t nq_transfer_size;     /* transfer size in bytes */
	u_int32_t nq_use_expensive:1,   /* allowed = 1 otherwise 0 */
	    nq_uplink:1,                /* uplink = 1 otherwise 0 */
	    nq_use_constrained:1;       /* allowed = 1 otherwise 0 */
	u_int32_t nq_unused;            /* for future expansion */
};

#ifndef KERNEL

/*
 * Returns whether a large upload or download transfer should be marked as
 * BK service type for network activity. This is a system level
 * hint/suggestion to classify application traffic based on statistics
 * collected from the current network attachment
 *
 *	@param	param	transfer parameters
 *	@param	param_len parameter length
 *	@return	returns 1 for BK and 0 for default
 */
extern int net_qos_guideline(struct net_qos_param *param, size_t param_len);

#endif /* !KERNEL */

#ifdef BSD_KERNEL_PRIVATE

extern int net_qos_policy_restricted;
extern int net_qos_policy_wifi_enabled;
extern int net_qos_policy_capable_enabled;

extern void net_qos_map_init(void);
extern void net_qos_map_change(uint32_t mode);
extern errno_t set_packet_qos(struct mbuf *, struct ifnet *, boolean_t, int,
    int, u_int8_t *);
extern int so_get_netsvc_marking_level(struct socket *);

extern uint8_t fastlane_sc_to_dscp(uint32_t svc_class);
extern uint8_t rfc4594_sc_to_dscp(uint32_t svc_class);
extern mbuf_traffic_class_t rfc4594_dscp_to_tc(uint8_t dscp);

#endif /* BSD_KERNEL_PRIVATE */

#endif /* PRIVATE */

#endif /* __NETINET_IN_TCLASS_H__ */
