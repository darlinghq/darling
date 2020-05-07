/*
 * Copyright (c) 2010-2018 Apple Inc. All rights reserved.
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
/*-
 * Copyright (c) 2008 Swinburne University of Technology, Melbourne, Australia
 * All rights reserved.
 *
 * This software was developed at the Centre for Advanced Internet
 * Architectures, Swinburne University, by Lawrence Stewart and James Healy,
 * made possible in part by a grant from the Cisco University Research Program
 * Fund at Community Foundation Silicon Valley.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#ifndef _NETINET_CC_H_
#define _NETINET_CC_H_

#ifdef PRIVATE

#include <netinet/in.h>

/*
 * Data structure to collect and display congestion control debug information
 */
struct tcp_cc_debug_state {
	u_int64_t ccd_tsns;
	char ccd_srcaddr[INET6_ADDRSTRLEN];
	uint16_t ccd_srcport;
	char ccd_destaddr[INET6_ADDRSTRLEN];
	uint16_t ccd_destport;
	uint32_t ccd_snd_cwnd;
	uint32_t ccd_snd_wnd;
	uint32_t ccd_snd_ssthresh;
	uint32_t ccd_pipeack;
	uint32_t ccd_rttcur;
	uint32_t ccd_rxtcur;
	uint32_t ccd_srtt;
	uint32_t ccd_event;
	uint32_t ccd_sndcc;
	uint32_t ccd_sndhiwat;
	uint32_t ccd_bytes_acked;
	u_int8_t ccd_cc_index;
	u_int8_t ccd_unused_1__;
	u_int16_t ccd_unused_2__;
	union {
		struct {
			uint32_t ccd_last_max;
			uint32_t ccd_tcp_win;
			uint32_t ccd_target_win;
			uint32_t ccd_avg_lastmax;
			uint32_t ccd_mean_deviation;
		} cubic_state;
		struct {
			u_int32_t led_base_rtt;
		} ledbat_state;
	} u;
};

/*
 * Values of ccd_cc_index
 */
#define TCP_CC_ALGO_NONE                0
#define TCP_CC_ALGO_NEWRENO_INDEX       1
#define TCP_CC_ALGO_BACKGROUND_INDEX    2 /* CC for background transport */
#define TCP_CC_ALGO_CUBIC_INDEX         3 /* default CC algorithm */
#define TCP_CC_ALGO_COUNT               4 /* Count of CC algorithms */

/*
 * Values of ccd_event
 */
#define TCP_CC_EVENT_LIST                       \
	X(TCP_CC_CWND_INIT)                     \
	X(TCP_CC_INSEQ_ACK_RCVD)                \
	X(TCP_CC_ACK_RCVD)                      \
	X(TCP_CC_ENTER_FASTRECOVERY)            \
	X(TCP_CC_IN_FASTRECOVERY)               \
	X(TCP_CC_EXIT_FASTRECOVERY)             \
	X(TCP_CC_PARTIAL_ACK)                   \
	X(TCP_CC_IDLE_TIMEOUT)                  \
	X(TCP_CC_REXMT_TIMEOUT)                 \
	X(TCP_CC_ECN_RCVD)                      \
	X(TCP_CC_BAD_REXMT_RECOVERY)            \
	X(TCP_CC_OUTPUT_ERROR)                  \
	X(TCP_CC_CHANGE_ALGO)                   \
	X(TCP_CC_FLOW_CONTROL)                  \
	X(TCP_CC_SUSPEND)                       \
	X(TCP_CC_LIMITED_TRANSMIT)              \
	X(TCP_CC_EARLY_RETRANSMIT)              \
	X(TCP_CC_TLP_RECOVERY)                  \
	X(TCP_CC_TLP_RECOVER_LASTPACKET)        \
	X(TCP_CC_DELAY_FASTRECOVERY)            \
	X(TCP_CC_TLP_IN_FASTRECOVERY)           \
	X(TCP_CC_DSACK_BAD_REXMT)               \
	X(TCP_CC_FIRST_REXMT)                   \
	X(MAX_TCP_CC_EVENTS)

enum tcp_cc_event {
#define X(name, ...) name,
	TCP_CC_EVENT_LIST
#undef X
};

/*
 * Kernel control ID
 */
#define TCP_CC_CONTROL_NAME     "com.apple.network.tcp_ccdebug"

#endif /* PRIVATE */

#ifdef KERNEL_PRIVATE

#include <netinet/tcp.h>
#include <netinet/tcp_var.h>
#include <kern/zalloc.h>

/*
 * Maximum characters in the name of a CC algorithm
 */
#define TCP_CA_NAME_MAX 16

extern int tcp_recv_bg;

/*
 * Structure to hold definition various actions defined by a congestion
 * control algorithm for TCP. This can be used to change the congestion
 * control on a connection based on the user settings of priority of a
 * connection.
 */
struct tcp_cc_algo {
	char name[TCP_CA_NAME_MAX];
	uint32_t num_sockets;
	uint32_t flags;

	/* init the congestion algorithm for the specified control block */
	int (*init) (struct tcpcb *tp);

	/*
	 * cleanup any state that is stored in the connection
	 * related to the algorithm
	 */
	int (*cleanup) (struct tcpcb *tp);

	/* initialize cwnd at the start of a connection */
	void (*cwnd_init) (struct tcpcb *tp);

	/*
	 * called on the receipt of in-sequence ack during congestion
	 * avoidance phase
	 */
	void (*congestion_avd) (struct tcpcb *tp, struct tcphdr *th);

	/* called on the receipt of a valid ack */
	void (*ack_rcvd) (struct tcpcb *tp, struct tcphdr *th);

	/* called before entering FR */
	void (*pre_fr) (struct tcpcb *tp);

	/*  after exiting FR */
	void (*post_fr) (struct tcpcb *tp, struct tcphdr *th);

	/* perform tasks when data transfer resumes after an idle period */
	void (*after_idle) (struct tcpcb *tp);

	/* perform tasks when the connection's retransmit timer expires */
	void (*after_timeout) (struct tcpcb *tp);

	/* Whether or not to delay the ack */
	int (*delay_ack)(struct tcpcb *tp, struct tcphdr *th);

	/* Switch a connection to this CC algorithm after sending some packets */
	void (*switch_to)(struct tcpcb *tp, uint16_t old_cc_index);
} __attribute__((aligned(4)));

extern struct zone *tcp_cc_zone;

extern struct tcp_cc_algo* tcp_cc_algo_list[TCP_CC_ALGO_COUNT];

#define CC_ALGO(tp) (tcp_cc_algo_list[tp->tcp_cc_index])
#define TCP_CC_CWND_INIT_BYTES  4380
/*
 * The congestion window will have to be reset after a
 * non-validated period -- currently set to 3 minutes
 */
#define TCP_CC_CWND_NONVALIDATED_PERIOD (3 * 60 * TCP_RETRANSHZ)

extern void     tcp_cc_init(void);
extern void tcp_cc_resize_sndbuf(struct tcpcb *tp);
extern void tcp_bad_rexmt_fix_sndbuf(struct tcpcb *tp);
extern void tcp_cc_cwnd_init_or_reset(struct tcpcb *tp);
extern int tcp_cc_delay_ack(struct tcpcb *tp, struct tcphdr *th);
extern void tcp_ccdbg_trace(struct tcpcb *tp, struct tcphdr *th,
    int32_t event);
extern void tcp_cc_allocate_state(struct tcpcb *tp);
extern void tcp_cc_after_idle_stretchack(struct tcpcb *tp);
extern uint32_t tcp_cc_is_cwnd_nonvalidated(struct tcpcb *tp);
extern void tcp_cc_adjust_nonvalidated_cwnd(struct tcpcb *tp);
extern u_int32_t tcp_get_max_pipeack(struct tcpcb *tp);
extern void tcp_clear_pipeack_state(struct tcpcb *tp);

#endif /* KERNEL_PRIVATE */
#endif /* _NETINET_CC_H_ */
