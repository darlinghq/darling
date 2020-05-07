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

#ifndef __FLOW_DIVERT_PROTO_H__
#define __FLOW_DIVERT_PROTO_H__

#define FLOW_DIVERT_CONTROL_NAME                "com.apple.flow-divert"

#define FLOW_DIVERT_TLV_LENGTH_UINT32   1

#define FLOW_DIVERT_PKT_CONNECT                 1
#define FLOW_DIVERT_PKT_CONNECT_RESULT          2
#define FLOW_DIVERT_PKT_DATA                    3
#define FLOW_DIVERT_PKT_CLOSE                   4
#define FLOW_DIVERT_PKT_READ_NOTIFY             5
#define FLOW_DIVERT_PKT_GROUP_INIT              6
#define FLOW_DIVERT_PKT_PROPERTIES_UPDATE       7
#define FLOW_DIVERT_PKT_APP_MAP_CREATE          9

#define FLOW_DIVERT_TLV_NIL                     0
#define FLOW_DIVERT_TLV_ERROR_CODE              5
#define FLOW_DIVERT_TLV_HOW                     7
#define FLOW_DIVERT_TLV_READ_COUNT              8
#define FLOW_DIVERT_TLV_SPACE_AVAILABLE         9
#define FLOW_DIVERT_TLV_CTL_UNIT                10
#define FLOW_DIVERT_TLV_LOCAL_ADDR              11
#define FLOW_DIVERT_TLV_REMOTE_ADDR             12
#define FLOW_DIVERT_TLV_OUT_IF_INDEX            13
#define FLOW_DIVERT_TLV_TRAFFIC_CLASS           14
#define FLOW_DIVERT_TLV_NO_CELLULAR             15
#define FLOW_DIVERT_TLV_FLOW_ID                 16
#define FLOW_DIVERT_TLV_TOKEN_KEY               17
#define FLOW_DIVERT_TLV_HMAC                    18
#define FLOW_DIVERT_TLV_KEY_UNIT                19
#define FLOW_DIVERT_TLV_LOG_LEVEL               20
#define FLOW_DIVERT_TLV_TARGET_HOSTNAME         21
#define FLOW_DIVERT_TLV_TARGET_ADDRESS          22
#define FLOW_DIVERT_TLV_TARGET_PORT             23
#define FLOW_DIVERT_TLV_CDHASH                  24
#define FLOW_DIVERT_TLV_SIGNING_ID              25
#define FLOW_DIVERT_TLV_PID                     26
#define FLOW_DIVERT_TLV_UUID                    27
#define FLOW_DIVERT_TLV_PREFIX_COUNT            28
#define FLOW_DIVERT_TLV_FLAGS                   29
#define FLOW_DIVERT_TLV_FLOW_TYPE               30
#define FLOW_DIVERT_TLV_APP_DATA                31
#define FLOW_DIVERT_TLV_APP_AUDIT_TOKEN         32

#define FLOW_DIVERT_FLOW_TYPE_TCP               1
#define FLOW_DIVERT_FLOW_TYPE_UDP               3

#define FLOW_DIVERT_CHUNK_SIZE                  4096

#define FLOW_DIVERT_TOKEN_GETOPT_MAX_SIZE       128

#define FLOW_DIVERT_TOKEN_FLAG_VALIDATED        0x0000001
#define FLOW_DIVERT_TOKEN_FLAG_TFO                      0x0000002
#define FLOW_DIVERT_TOKEN_FLAG_MPTCP            0x0000004

#define FLOW_DIVERT_GROUP_FLAG_NO_APP_MAP       0x0000001

struct flow_divert_packet_header {
	uint8_t             packet_type;
	uint32_t            conn_id;
};

#endif /* __FLOW_DIVERT_PROTO_H__ */
