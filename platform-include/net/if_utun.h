/*
 * Copyright (c) 2008-2011 Apple Inc. All rights reserved.
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


#ifndef	_NET_IF_UTUN_H_
#define	_NET_IF_UTUN_H_

#include <net/if_utun_crypto.h>


/*
 * Name registered by the utun kernel control
 */
#define UTUN_CONTROL_NAME "com.apple.net.utun_control"

/*
 * Socket option names to manage utun
 */
#define UTUN_OPT_FLAGS							1
#define UTUN_OPT_IFNAME							2
#define UTUN_OPT_EXT_IFDATA_STATS				3	/* get|set (type int) */
#define UTUN_OPT_INC_IFDATA_STATS_IN			4	/* set to increment stat counters (type struct utun_stats_param) */ 
#define UTUN_OPT_INC_IFDATA_STATS_OUT			5	/* set to increment stat counters (type struct utun_stats_param) */ 
#define UTUN_OPT_ENABLE_CRYPTO					6
#define UTUN_OPT_CONFIG_CRYPTO_KEYS				7
#define UTUN_OPT_UNCONFIG_CRYPTO_KEYS			8
#define UTUN_OPT_GENERATE_CRYPTO_KEYS_IDX		9
#define UTUN_OPT_DISABLE_CRYPTO					10
#define UTUN_OPT_STOP_CRYPTO_DATA_TRAFFIC		11
#define UTUN_OPT_START_CRYPTO_DATA_TRAFFIC		12

/*
 * Flags for by UTUN_OPT_FLAGS 
 */
#define	UTUN_FLAGS_NO_OUTPUT		0x0001
#define UTUN_FLAGS_NO_INPUT			0x0002
#define UTUN_FLAGS_CRYPTO			0x0004
#define UTUN_FLAGS_CRYPTO_STOP_DATA_TRAFFIC	0x0008

/*
 * utun packet type flags
 */
#define UTUN_PKT_TYPE_KEEPALIVE					0x0001
#define UTUN_PKT_TYPE_IPSEC						0x0002
#define UTUN_PKT_TYPE_DTLS						0x0004


/*
 * utun stats parameter structure
 */
struct utun_stats_param {
        u_int64_t       utsp_packets;
        u_int64_t       utsp_bytes;
        u_int64_t       utsp_errors;
};

#endif
