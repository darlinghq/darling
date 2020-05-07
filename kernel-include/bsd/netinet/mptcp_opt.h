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

#ifndef _NETINET_MPTCP_OPT_H_
#define _NETINET_MPTCP_OPT_H_

#ifdef BSD_KERNEL_PRIVATE

__BEGIN_DECLS
extern void mptcp_data_ack_rcvd(struct mptcb *mp_tp, struct tcpcb *tp, u_int64_t full_dack);
extern void mptcp_update_window_wakeup(struct tcpcb *tp);
extern void tcp_do_mptcp_options(struct tcpcb *, u_char *, struct tcphdr *,
    struct tcpopt *, int);
extern unsigned mptcp_setup_syn_opts(struct socket *, u_char*, unsigned);
extern unsigned mptcp_setup_join_ack_opts(struct tcpcb *, u_char*, unsigned);
extern unsigned int mptcp_setup_opts(struct tcpcb *tp, int32_t off, u_char *opt,
    unsigned int optlen, int flags, int len,
    boolean_t *p_mptcp_acknow);
extern void mptcp_update_dss_rcv_state(struct mptcp_dsn_opt *, struct tcpcb *,
    uint16_t);
extern void mptcp_update_rcv_state_meat(struct mptcb *, struct tcpcb *,
    u_int64_t, u_int32_t, u_int16_t, uint16_t);
__END_DECLS

#endif /* BSD_KERNEL_PRIVATE */
#endif /* !_NETINET_MPTCP_OPT_H_ */
