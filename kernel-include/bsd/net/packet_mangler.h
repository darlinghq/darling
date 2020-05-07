/*
 * Copyright (c) 2014 Apple Inc. All rights reserved.
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

#ifndef __PACKET_MANGLER_H__
#define __PACKET_MANGLER_H__

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <netinet/in.h>
#include <stdint.h>

#ifdef BSD_KERNEL_PRIVATE
#include <sys/mbuf.h>
#include <sys/socketvar.h>
#endif /* BSD_KERNEL_PRIVATE */

__BEGIN_DECLS

#ifdef PRIVATE

typedef enum {
	INOUT,
	IN,
	OUT
} Pkt_Mnglr_Flow;

/*
 * Kernel control name for an instance of a packet mangler.
 * Use CTLIOCGINFO to find out the corresponding kernel control id
 * to be set in the sc_id field of sockaddr_ctl for connect(2)
 * Note: the sc_unit is ephemeral
 */
#define PACKET_MANGLER_CONTROL_NAME "com.apple.packet-mangler"

#define PKT_MNGLR_OPT_PROTO_ACT_MASK    1
#define PKT_MNGLR_OPT_IP_ACT_MASK       2
#define PKT_MNGLR_OPT_LOCAL_IP          3
#define PKT_MNGLR_OPT_REMOTE_IP         4
#define PKT_MNGLR_OPT_LOCAL_PORT        5
#define PKT_MNGLR_OPT_REMOTE_PORT       6
#define PKT_MNGLR_OPT_DIRECTION         7
#define PKT_MNGLR_OPT_PROTOCOL          8
#define PKT_MNGLR_OPT_ACTIVATE          0xFFFFFFFF

/* Packet mangler action masks */
/* Packet Mangler TCP action mask */
#define PKT_MNGLR_TCP_ACT_NOP_MPTCP     0x00000001
#define PKT_MNGLR_TCP_ACT_SWAP_L_PORT   0x00000002
#define PKT_MNGLR_TCP_ACT_SWAP_R_PORT   0x00000004
#define PKT_MNGLR_TCP_ACT_DSS_DROP      0x00000008
#define PKT_MNGLR_TCP_ACT_CHK_EXTENDED  0x80000000

/* Packet Mangler IP action mask */
#define PKT_MNGLR_IP_ACT_FLT_L_IP       0x00000001
#define PKT_MNGLR_IP_ACT_FLT_R_IP       0x00000002
#define PKT_MNGLR_IP_ACT_SWAP_L_IP      0x00000004
#define PKT_MNGLR_IP_ACT_SWAP_R_IP      0x00000008
#define PKT_MNGLR_IP_ACT_DROP_PACKET    0x00000010
#define PKT_MNGLR_IP_ACT_CHK_EXTENDED   0x80000000

/*
 * How many filter may be active simultaneously
 */
#define PKT_MNGLR_MAX_FILTER_COUNT      1

#define PKT_MNGLR_VERSION_CURRENT 1

#endif /* PRIVATE */

#ifdef BSD_KERNEL_PRIVATE

extern int pkt_mnglr_log_level;

#define PKT_MNGLR_LOG(level, fmt, ...) \
do { \
	if (pkt_mnglr_log_level >= level) \
	        printf("%s:%d " fmt "\n",\
	                __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while (0)


extern void pkt_mnglr_init(void);

__END_DECLS

#endif /* BSD_KERNEL_PRIVATE */

#endif /* __PACKET_MANGLER_H__ */
