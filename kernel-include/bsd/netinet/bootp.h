/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
/*
 * Bootstrap Protocol (BOOTP).  RFC 951.
 */
/*
 * HISTORY
 *
 * 14 May 1992 ? at NeXT
 *	Added correct padding to struct nextvend.  This is
 *	needed for the i386 due to alignment differences wrt
 *	the m68k.  Also adjusted the size of the array fields
 *	because the NeXT vendor area was overflowing the bootp
 *	packet.
 */

#include <netinet/udp.h>

#define iaddr_t struct in_addr

struct bootp {
	u_char  bp_op;          /* packet opcode type */
#define BOOTREQUEST     1
#define BOOTREPLY       2
	u_char  bp_htype;       /* hardware addr type */
	u_char  bp_hlen;        /* hardware addr length */
	u_char  bp_hops;        /* gateway hops */
	u_int32_t bp_xid;       /* transaction ID */
	u_short bp_secs;        /* seconds since boot began */
	u_short bp_unused;
	iaddr_t bp_ciaddr;      /* client IP address */
	iaddr_t bp_yiaddr;      /* 'your' IP address */
	iaddr_t bp_siaddr;      /* server IP address */
	iaddr_t bp_giaddr;      /* gateway IP address */
	u_char  bp_chaddr[16];  /* client hardware address */
	u_char  bp_sname[64];   /* server host name */
	u_char  bp_file[128];   /* boot file name */
	u_char  bp_vend[64];    /* vendor-specific area */
};

/*
 * UDP port numbers, server and client.
 */
#define IPPORT_BOOTPS           67
#define IPPORT_BOOTPC           68

/*
 * "vendor" data permitted for Stanford boot clients.
 */
struct vend {
	u_char  v_magic[4];     /* magic number */
	u_int32_t v_flags;      /* flags/opcodes, etc. */
	u_char  v_unused[56];   /* currently unused */
};
#define VM_STANFORD     "STAN"  /* v_magic for Stanford */

/* v_flags values */
#define VF_PCBOOT       1       /* an IBMPC or Mac wants environment info */
#define VF_HELP         2       /* help me, I'm not registered */

#define NVMAXTEXT       55      /* don't change this, it just fits RFC951 */
struct nextvend {
	u_char nv_magic[4];     /* Magic number for vendor specificity */
	u_char nv_version;      /* NeXT protocol version */
	/*
	 * Round the beginning
	 * of the union to a 16
	 * bit boundary due to
	 * struct/union alignment
	 * on the m68k.
	 */
	unsigned short  :0;
	union {
		u_char NV0[58];
		struct {
			u_char NV1_opcode;      /* opcode - Version 1 */
			u_char NV1_xid; /* transcation id */
			u_char NV1_text[NVMAXTEXT];     /* text */
			u_char NV1_null;        /* null terminator */
		} NV1;
	} nv_U;
};
#define nv_unused       nv_U.NV0
#define nv_opcode       nv_U.NV1.NV1_opcode
#define nv_xid          nv_U.NV1.NV1_xid
#define nv_text         nv_U.NV1.NV1_text
#define nv_null         nv_U.NV1.NV1_null

/* Magic number */
#define VM_NEXT         "NeXT"  /* v_magic for NeXT, Inc. */

/* Opcodes */
#define BPOP_OK         0
#define BPOP_QUERY      1
#define BPOP_QUERY_NE   2
#define BPOP_ERROR      3

struct bootp_packet {
	struct ip bp_ip;
	struct udphdr bp_udp;
	struct bootp bp_bootp;
};

#define BOOTP_PKTSIZE (sizeof (struct bootp_packet))

/* backoffs must be masks */
#define BOOTP_MIN_BACKOFF       0x7ff           /* 2.048 sec */
#define BOOTP_MAX_BACKOFF       0xffff          /* 65.535 sec */
#define BOOTP_RETRY             6               /* # retries */
