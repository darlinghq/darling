/*
 * Copyright (c) 2008-2013 Apple Inc. All rights reserved.
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
 * Copyright (c) 1993 Daniel Boulet
 * Copyright (c) 1994 Ugen J.S.Antsilevich
 *
 * Redistribution and use in source forms, with and without modification,
 * are permitted provided that this entire comment appears intact.
 *
 * Redistribution in binary form may occur without any restrictions.
 * Obviously, it would be nice if you gave credit where credit is due
 * but requiring it would be too onerous.
 *
 * This software is provided ``AS IS'' without any warranties of any kind.
 *
 */

#ifndef _IP6_FW_H
#define _IP6_FW_H
#ifdef __APPLE_API_OBSOLETE

#include <sys/appleapiopts.h>

/*
 * Define IPv6 Firewall event subclass, and associated events.
 */

/*!
 *       @defined KEV_IP6FW_SUBCLASS
 *       @discussion The kernel event subclass for IPv6 Firewall.
 */
#define KEV_IP6FW_SUBCLASS      2

/*!
 *       @defined KEV_IP6FW_ADD
 *       @discussion The event code indicating a rule has been added.
 */
#define KEV_IP6FW_ADD           1

/*!
 *       @defined KEV_IP6FW_DEL
 *       @discussion The event code indicating a rule has been removed.
 */
#define KEV_IP6FW_DEL           2

/*!
 *       @defined KEV_IP6FW_FLUSH
 *       @discussion The event code indicating the rule set has been flushed.
 */
#define KEV_IP6FW_FLUSH         3

/*!
 *       @defined KEV_IP6FW_FLUSH
 *       @discussion The event code indicating the enable flag has been changed
 */
#define KEV_IP6FW_ENABLE        4


#include <net/if.h>

#define IPV6_FW_CURRENT_API_VERSION 20  /* Version of this API */


/*
 * This union structure identifies an interface, either explicitly
 * by name or implicitly by IP address. The flags IP_FW_F_IIFNAME
 * and IP_FW_F_OIFNAME say how to interpret this structure. An
 * interface unit number of -1 matches any unit number, while an
 * IP address of 0.0.0.0 indicates matches any interface.
 *
 * The receive and transmit interfaces are only compared against the
 * the packet if the corresponding bit (IP_FW_F_IIFACE or IP_FW_F_OIFACE)
 * is set. Note some packets lack a receive or transmit interface
 * (in which case the missing "interface" never matches).
 */

union ip6_fw_if {
	struct in6_addr fu_via_ip6; /* Specified by IPv6 address */
	struct {                /* Specified by interface name */
#define IP6FW_IFNLEN     IFNAMSIZ
		char  name[IP6FW_IFNLEN];
		short unit;     /* -1 means match any unit */
	} fu_via_if;
};

/*
 * Format of an IP firewall descriptor
 *
 * fw_src, fw_dst, fw_smsk, fw_dmsk are always stored in network byte order.
 * fw_flg and fw_n*p are stored in host byte order (of course).
 * Port numbers are stored in HOST byte order.
 * Warning: setsockopt() will fail if sizeof(struct ip_fw) > MLEN (108)
 */


struct ip6_fw {
	u_int32_t version;              /* Version of this structure.  Should always be */
	/* set to IP6_FW_CURRENT_API_VERSION by clients. */
	void *context;                  /* Context that is usable by user processes to */
	/* identify this rule. */
	u_int32_t fw_pcnt, fw_bcnt;     /* Packet and byte counters */
	struct in6_addr fw_src, fw_dst; /* Source and destination IPv6 addr */
	struct in6_addr fw_smsk, fw_dmsk; /* Mask for src and dest IPv6 addr */
	u_short fw_number;              /* Rule number */
	u_short fw_flg;                 /* Flags word */
#define IPV6_FW_MAX_PORTS       10      /* A reasonable maximum */
	u_int fw_ipflg;                 /* IP flags word */
	u_short fw_pts[IPV6_FW_MAX_PORTS]; /* Array of port numbers to match */
	u_char fw_ip6opt, fw_ip6nopt;   /* IPv6 options set/unset */
	u_char fw_tcpf, fw_tcpnf;       /* TCP flags set/unset */
#define IPV6_FW_ICMPTYPES_DIM (256 / (sizeof(unsigned) * 8))
	unsigned fw_icmp6types[IPV6_FW_ICMPTYPES_DIM]; /* ICMP types bitmap */
	long timestamp;                 /* timestamp (tv_sec) of last match */
	union ip6_fw_if fw_in_if, fw_out_if;/* Incoming and outgoing interfaces */
	union {
		u_short fu_divert_port; /* Divert/tee port (options IP6DIVERT) */
		u_short fu_skipto_rule; /* SKIPTO command rule number */
		u_short fu_reject_code; /* REJECT response code */
	} fw_un;
	u_char fw_prot;                 /* IPv6 protocol */
	u_char fw_nports;               /* N'of src ports and # of dst ports */
	                                /* in ports array (dst ports follow */
	                                /* src ports; max of 10 ports in all; */
	                                /* count of 0 means match all ports) */
};

#if defined(BSD_KERNEL_PRIVATE)
#pragma pack(4)

struct ip6_fw_32 {
	u_int32_t version;              /* Version of this structure.  Should always be */
	/* set to IP6_FW_CURRENT_API_VERSION by clients. */
	user32_addr_t context;                  /* Context that is usable by user processes to */
	/* identify this rule. */
	u_int32_t fw_pcnt, fw_bcnt;     /* Packet and byte counters */
	struct in6_addr fw_src, fw_dst; /* Source and destination IPv6 addr */
	struct in6_addr fw_smsk, fw_dmsk; /* Mask for src and dest IPv6 addr */
	u_short fw_number;              /* Rule number */
	u_short fw_flg;                 /* Flags word */
#define IPV6_FW_MAX_PORTS       10      /* A reasonable maximum */
	u_int fw_ipflg;                 /* IP flags word */
	u_short fw_pts[IPV6_FW_MAX_PORTS]; /* Array of port numbers to match */
	u_char fw_ip6opt, fw_ip6nopt;   /* IPv6 options set/unset */
	u_char fw_tcpf, fw_tcpnf;       /* TCP flags set/unset */
#define IPV6_FW_ICMPTYPES_DIM (256 / (sizeof(unsigned) * 8))
	unsigned fw_icmp6types[IPV6_FW_ICMPTYPES_DIM]; /* ICMP types bitmap */
	user32_time_t timestamp;                /* timestamp (tv_sec) of last match */
	union ip6_fw_if fw_in_if, fw_out_if;/* Incoming and outgoing interfaces */
	union {
		u_short fu_divert_port;         /* Divert/tee port (options IP6DIVERT) */
		u_short fu_skipto_rule;         /* SKIPTO command rule number */
		u_short fu_reject_code;         /* REJECT response code */
	} fw_un;
	u_char fw_prot;                 /* IPv6 protocol */
	u_char fw_nports;               /* N'of src ports and # of dst ports */
	/* in ports array (dst ports follow */
	/* src ports; max of 10 ports in all; */
	/* count of 0 means match all ports) */
};

#pragma pack()

struct ip6_fw_64 {
	u_int32_t version;              /* Version of this structure.  Should always be */
	/* set to IP6_FW_CURRENT_API_VERSION by clients. */
	__uint64_t context __attribute__((aligned(8)));                 /* Context that is usable by user processes to */
	/* identify this rule. */
	u_int32_t fw_pcnt, fw_bcnt;     /* Packet and byte counters */
	struct in6_addr fw_src, fw_dst; /* Source and destination IPv6 addr */
	struct in6_addr fw_smsk, fw_dmsk; /* Mask for src and dest IPv6 addr */
	u_short fw_number;              /* Rule number */
	u_short fw_flg;                 /* Flags word */
#define IPV6_FW_MAX_PORTS       10      /* A reasonable maximum */
	u_int fw_ipflg;                 /* IP flags word */
	u_short fw_pts[IPV6_FW_MAX_PORTS]; /* Array of port numbers to match */
	u_char fw_ip6opt, fw_ip6nopt;   /* IPv6 options set/unset */
	u_char fw_tcpf, fw_tcpnf;       /* TCP flags set/unset */
#define IPV6_FW_ICMPTYPES_DIM (256 / (sizeof(unsigned) * 8))
	unsigned fw_icmp6types[IPV6_FW_ICMPTYPES_DIM]; /* ICMP types bitmap */
	user64_time_t timestamp;                /* timestamp (tv_sec) of last match */
	union ip6_fw_if fw_in_if, fw_out_if;/* Incoming and outgoing interfaces */
	union {
		u_short fu_divert_port;         /* Divert/tee port (options IP6DIVERT) */
		u_short fu_skipto_rule;         /* SKIPTO command rule number */
		u_short fu_reject_code;         /* REJECT response code */
	} fw_un;
	u_char fw_prot;                 /* IPv6 protocol */
	u_char fw_nports;               /* N'of src ports and # of dst ports */
	/* in ports array (dst ports follow */
	/* src ports; max of 10 ports in all; */
	/* count of 0 means match all ports) */
};


#endif  /* BSD_KERNEL_PRIVATE */

#define IPV6_FW_GETNSRCP(rule)          ((rule)->fw_nports & 0x0f)
#define IPV6_FW_SETNSRCP(rule, n)               do {                            \
	                                  (rule)->fw_nports &= ~0x0f;   \
	                                  (rule)->fw_nports |= (n);     \
	                                } while (0)
#define IPV6_FW_GETNDSTP(rule)          ((rule)->fw_nports >> 4)
#define IPV6_FW_SETNDSTP(rule, n)               do {                            \
	                                  (rule)->fw_nports &= ~0xf0;   \
	                                  (rule)->fw_nports |= (n) << 4;\
	                                } while (0)

#define fw_divert_port  fw_un.fu_divert_port
#define fw_skipto_rule  fw_un.fu_skipto_rule
#define fw_reject_code  fw_un.fu_reject_code

struct ip6_fw_chain {
	LIST_ENTRY(ip6_fw_chain) chain;
	struct ip6_fw    *rule;
};

/*
 * Values for "flags" field .
 */
#define IPV6_FW_F_IN    0x0001  /* Check inbound packets		*/
#define IPV6_FW_F_OUT   0x0002  /* Check outbound packets		*/
#define IPV6_FW_F_IIFACE        0x0004  /* Apply inbound interface test		*/
#define IPV6_FW_F_OIFACE        0x0008  /* Apply outbound interface test	*/

#define IPV6_FW_F_COMMAND 0x0070        /* Mask for type of chain entry:	*/
#define IPV6_FW_F_DENY  0x0000  /* This is a deny rule			*/
#define IPV6_FW_F_REJECT        0x0010  /* Deny and send a response packet	*/
#define IPV6_FW_F_ACCEPT        0x0020  /* This is an accept rule		*/
#define IPV6_FW_F_COUNT 0x0030  /* This is a count rule			*/
#define IPV6_FW_F_DIVERT        0x0040  /* This is a divert rule		*/
#define IPV6_FW_F_TEE   0x0050  /* This is a tee rule			*/
#define IPV6_FW_F_SKIPTO        0x0060  /* This is a skipto rule		*/

#define IPV6_FW_F_PRN   0x0080  /* Print if this rule matches		*/

#define IPV6_FW_F_SRNG  0x0100  /* The first two src ports are a min	*
	                         * and max range (stored in host byte	*
	                         * order).				*/

#define IPV6_FW_F_DRNG  0x0200  /* The first two dst ports are a min	*
	                         * and max range (stored in host byte	*
	                         * order).				*/

#define IPV6_FW_F_IIFNAME       0x0400  /* In interface by name/unit (not IP)	*/
#define IPV6_FW_F_OIFNAME       0x0800  /* Out interface by name/unit (not IP)	*/

#define IPV6_FW_F_INVSRC        0x1000  /* Invert sense of src check		*/
#define IPV6_FW_F_INVDST        0x2000  /* Invert sense of dst check		*/

#define IPV6_FW_F_FRAG  0x4000  /* Fragment				*/

#define IPV6_FW_F_ICMPBIT 0x8000        /* ICMP type bitmap is valid		*/

#define IPV6_FW_F_MASK  0xFFFF  /* All possible flag bits mask		*/

/*
 * Flags for the 'fw_ipflg' field, for comparing values of ip and its protocols. */
#define IPV6_FW_IF_TCPEST 0x00000020    /* established TCP connection	*/
#define IPV6_FW_IF_TCPMSK 0x00000020    /* mask of all TCP values */

/*
 * For backwards compatibility with rules specifying "via iface" but
 * not restricted to only "in" or "out" packets, we define this combination
 * of bits to represent this configuration.
 */

#define IF6_FW_F_VIAHACK        (IPV6_FW_F_IN|IPV6_FW_F_OUT|IPV6_FW_F_IIFACE|IPV6_FW_F_OIFACE)

/*
 * Definitions for REJECT response codes.
 * Values less than 256 correspond to ICMP unreachable codes.
 */
#define IPV6_FW_REJECT_RST      0x0100          /* TCP packets: send RST */

/*
 * Definitions for IPv6 option names.
 */
#define IPV6_FW_IP6OPT_HOPOPT   0x01
#define IPV6_FW_IP6OPT_ROUTE    0x02
#define IPV6_FW_IP6OPT_FRAG     0x04
#define IPV6_FW_IP6OPT_ESP      0x08
#define IPV6_FW_IP6OPT_AH       0x10
#define IPV6_FW_IP6OPT_NONXT    0x20
#define IPV6_FW_IP6OPT_OPTS     0x40

/*
 * Definitions for TCP flags.
 */
#define IPV6_FW_TCPF_FIN        TH_FIN
#define IPV6_FW_TCPF_SYN        TH_SYN
#define IPV6_FW_TCPF_RST        TH_RST
#define IPV6_FW_TCPF_PSH        TH_PUSH
#define IPV6_FW_TCPF_ACK        TH_ACK
#define IPV6_FW_TCPF_URG        TH_URG

/*
 * Main firewall chains definitions and global var's definitions.
 */
#ifdef BSD_KERNEL_PRIVATE

#define M_IP6FW M_IPFW


/*
 * Function definitions.
 */
void ip6_fw_init(void);
void load_ip6fw(void);

/* Firewall hooks */
struct ip6_hdr;
struct sockopt;
typedef int ip6_fw_chk_t(struct ip6_hdr**, struct ifnet*,
    u_short *, struct mbuf**);
typedef int ip6_fw_ctl_t(struct sockopt *);
extern  ip6_fw_chk_t *ip6_fw_chk_ptr;
extern  ip6_fw_ctl_t *ip6_fw_ctl_ptr;
extern  int ip6_fw_enable;

#endif /* BSD_KERNEL_PRIVATE */

#endif /* __APPLE_API_OBSOLETE */
#endif /* _IP6_FW_H */
