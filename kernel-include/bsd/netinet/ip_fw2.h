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
 * Copyright (c) 2002 Luigi Rizzo, Universita` di Pisa
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
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/sys/netinet/ip_fw2.h,v 1.1.2.4 2003/07/17 06:03:39 luigi Exp $
 */

#ifndef _IPFW2_H
#define _IPFW2_H
#ifdef __APPLE_API_OBSOLETE

/*
 * Define IP Firewall event subclass, and associated events.
 */

/*!
 *       @defined KEV_IPFW_SUBCLASS
 *       @discussion The kernel event subclass for IP Firewall.
 */
#define KEV_IPFW_SUBCLASS       1

/*!
 *       @defined KEV_IPFW_ADD
 *       @discussion The event code indicating a rule has been added.
 */
#define KEV_IPFW_ADD            1

/*!
 *       @defined KEV_IPFW_DEL
 *       @discussion The event code indicating a rule has been removed.
 */
#define KEV_IPFW_DEL            2

/*!
 *       @defined KEV_IPFW_FLUSH
 *       @discussion The event code indicating the rule set has been flushed.
 */
#define KEV_IPFW_FLUSH          3

/*!
 *       @defined KEV_IPFW_ENABLE
 *       @discussion The event code indicating the enable flag has been changed
 */
#define KEV_IPFW_ENABLE         4



/*
 * The kernel representation of ipfw rules is made of a list of
 * 'instructions' (for all practical purposes equivalent to BPF
 * instructions), which specify which fields of the packet
 * (or its metadata) should be analysed.
 *
 * Each instruction is stored in a structure which begins with
 * "ipfw_insn", and can contain extra fields depending on the
 * instruction type (listed below).
 * Note that the code is written so that individual instructions
 * have a size which is a multiple of 32 bits. This means that, if
 * such structures contain pointers or other 64-bit entities,
 * (there is just one instance now) they may end up unaligned on
 * 64-bit architectures, so the must be handled with care.
 *
 * "enum ipfw_opcodes" are the opcodes supported. We can have up
 * to 256 different opcodes.
 */

enum ipfw_opcodes {             /* arguments (4 byte each)	*/
	O_NOP,

	O_IP_SRC,               /* u32 = IP			*/
	O_IP_SRC_MASK,          /* ip = IP/mask			*/
	O_IP_SRC_ME,            /* none				*/
	O_IP_SRC_SET,           /* u32=base, arg1=len, bitmap	*/

	O_IP_DST,               /* u32 = IP			*/
	O_IP_DST_MASK,          /* ip = IP/mask			*/
	O_IP_DST_ME,            /* none				*/
	O_IP_DST_SET,           /* u32=base, arg1=len, bitmap	*/

	O_IP_SRCPORT,           /* (n)port list:mask 4 byte ea	*/
	O_IP_DSTPORT,           /* (n)port list:mask 4 byte ea	*/
	O_PROTO,                /* arg1=protocol		*/

	O_MACADDR2,             /* 2 mac addr:mask		*/
	O_MAC_TYPE,             /* same as srcport		*/

	O_LAYER2,               /* none				*/
	O_IN,                   /* none				*/
	O_FRAG,                 /* none				*/

	O_RECV,                 /* none				*/
	O_XMIT,                 /* none				*/
	O_VIA,                  /* none				*/

	O_IPOPT,                /* arg1 = 2*u8 bitmap		*/
	O_IPLEN,                /* arg1 = len			*/
	O_IPID,                 /* arg1 = id			*/

	O_IPTOS,                /* arg1 = id			*/
	O_IPPRECEDENCE,         /* arg1 = precedence << 5	*/
	O_IPTTL,                /* arg1 = TTL			*/

	O_IPVER,                /* arg1 = version		*/
	O_UID,                  /* u32 = id			*/
	O_GID,                  /* u32 = id			*/
	O_ESTAB,                /* none (tcp established)	*/
	O_TCPFLAGS,             /* arg1 = 2*u8 bitmap		*/
	O_TCPWIN,               /* arg1 = desired win		*/
	O_TCPSEQ,               /* u32 = desired seq.		*/
	O_TCPACK,               /* u32 = desired seq.		*/
	O_ICMPTYPE,             /* u32 = icmp bitmap		*/
	O_TCPOPTS,              /* arg1 = 2*u8 bitmap		*/

	O_VERREVPATH,           /* none				*/

	O_PROBE_STATE,          /* none				*/
	O_KEEP_STATE,           /* none				*/
	O_LIMIT,                /* ipfw_insn_limit		*/
	O_LIMIT_PARENT,         /* dyn_type, not an opcode.	*/

	/*
	 * These are really 'actions'.
	 */

	O_LOG,                  /* ipfw_insn_log		*/
	O_PROB,                 /* u32 = match probability	*/

	O_CHECK_STATE,          /* none				*/
	O_ACCEPT,               /* none				*/
	O_DENY,                 /* none                         */
	O_REJECT,               /* arg1=icmp arg (same as deny)	*/
	O_COUNT,                /* none				*/
	O_SKIPTO,               /* arg1=next rule number	*/
	O_PIPE,                 /* arg1=pipe number		*/
	O_QUEUE,                /* arg1=queue number		*/
	O_DIVERT,               /* arg1=port number		*/
	O_TEE,                  /* arg1=port number		*/
	O_FORWARD_IP,           /* fwd sockaddr			*/
	O_FORWARD_MAC,          /* fwd mac			*/

	/*
	 * More opcodes.
	 */
	O_IPSEC,                /* has ipsec history		*/

	O_LAST_OPCODE           /* not an opcode!		*/
};

/*
 * Template for instructions.
 *
 * ipfw_insn is used for all instructions which require no operands,
 * a single 16-bit value (arg1), or a couple of 8-bit values.
 *
 * For other instructions which require different/larger arguments
 * we have derived structures, ipfw_insn_*.
 *
 * The size of the instruction (in 32-bit words) is in the low
 * 6 bits of "len". The 2 remaining bits are used to implement
 * NOT and OR on individual instructions. Given a type, you can
 * compute the length to be put in "len" using F_INSN_SIZE(t)
 *
 * F_NOT	negates the match result of the instruction.
 *
 * F_OR		is used to build or blocks. By default, instructions
 *		are evaluated as part of a logical AND. An "or" block
 *		{ X or Y or Z } contains F_OR set in all but the last
 *		instruction of the block. A match will cause the code
 *		to skip past the last instruction of the block.
 *
 * NOTA BENE: in a couple of places we assume that
 *	sizeof(ipfw_insn) == sizeof(u_int32_t)
 * this needs to be fixed.
 *
 */
typedef struct  _ipfw_insn {    /* template for instructions */
	enum ipfw_opcodes       opcode:8;
	u_int8_t        len;    /* numer of 32-byte words */
#define F_NOT           0x80
#define F_OR            0x40
#define F_LEN_MASK      0x3f
#define F_LEN(cmd)      ((cmd)->len & F_LEN_MASK)

	u_int16_t       arg1;
} ipfw_insn;

/*
 * The F_INSN_SIZE(type) computes the size, in 4-byte words, of
 * a given type.
 */
#define F_INSN_SIZE(t)  ((sizeof (t))/sizeof(u_int32_t))

/*
 * This is used to store an array of 16-bit entries (ports etc.)
 */
typedef struct  _ipfw_insn_u16 {
	ipfw_insn o;
	u_int16_t ports[2];     /* there may be more */
} ipfw_insn_u16;

/*
 * This is used to store an array of 32-bit entries
 * (uid, single IPv4 addresses etc.)
 */
typedef struct  _ipfw_insn_u32 {
	ipfw_insn o;
	u_int32_t d[1]; /* one or more */
} ipfw_insn_u32;

/*
 * This is used to store IP addr-mask pairs.
 */
typedef struct  _ipfw_insn_ip {
	ipfw_insn o;
	struct in_addr  addr;
	struct in_addr  mask;
} ipfw_insn_ip;

/*
 * This is used to forward to a given address (ip).
 */
typedef struct  _ipfw_insn_sa {
	ipfw_insn o;
	struct sockaddr_in sa;
} ipfw_insn_sa;

/*
 * This is used for MAC addr-mask pairs.
 */
typedef struct  _ipfw_insn_mac {
	ipfw_insn o;
	u_char addr[12];        /* dst[6] + src[6] */
	u_char mask[12];        /* dst[6] + src[6] */
} ipfw_insn_mac;

/*
 * This is used for interface match rules (recv xx, xmit xx).
 */
typedef struct  _ipfw_insn_if {
	ipfw_insn o;
	union {
		struct in_addr ip;
		int32_t unit;
	} p;
	char name[IFNAMSIZ];
} ipfw_insn_if;

/*
 * This is used for pipe and queue actions, which need to store
 * a single pointer (which can have different size on different
 * architectures.
 * Note that, because of previous instructions, pipe_ptr might
 * be unaligned in the overall structure, so it needs to be
 * manipulated with care.
 */
typedef struct  _ipfw_insn_pipe {
	ipfw_insn       o;
	void            *pipe_ptr;      /* XXX */
} ipfw_insn_pipe;

/*
 * This is used for limit rules.
 */
typedef struct  _ipfw_insn_limit {
	ipfw_insn o;
	u_int8_t _pad;
	u_int8_t limit_mask;    /* combination of DYN_* below	*/
#define DYN_SRC_ADDR    0x1
#define DYN_SRC_PORT    0x2
#define DYN_DST_ADDR    0x4
#define DYN_DST_PORT    0x8

	u_int16_t conn_limit;
} ipfw_insn_limit;

/*
 * This is used for log instructions.
 */
typedef struct  _ipfw_insn_log {
	ipfw_insn o;
	u_int32_t max_log;      /* how many do we log -- 0 = all */
	u_int32_t log_left;     /* how many left to log         */
} ipfw_insn_log;

/* Version of this API */
#define IP_FW_VERSION_NONE 0
#define IP_FW_VERSION_0 10      /* old ipfw */
#define IP_FW_VERSION_1 20      /* ipfw in Jaguar/Panther */
#define IP_FW_VERSION_2 30      /* ipfw2 */
#define IP_FW_CURRENT_API_VERSION IP_FW_VERSION_2

/*
 * Here we have the structure representing an ipfw rule.
 *
 * It starts with a general area (with link fields and counters)
 * followed by an array of one or more instructions, which the code
 * accesses as an array of 32-bit values.
 *
 * Given a rule pointer  r:
 *
 *  r->cmd		is the start of the first instruction.
 *  ACTION_PTR(r)	is the start of the first action (things to do
 *			once a rule matched).
 *
 * When assembling instruction, remember the following:
 *
 *  + if a rule has a "keep-state" (or "limit") option, then the
 *	first instruction (at r->cmd) MUST BE an O_PROBE_STATE
 *  + if a rule has a "log" option, then the first action
 *	(at ACTION_PTR(r)) MUST be O_LOG
 *
 * NOTE: we use a simple linked list of rules because we never need
 *      to delete a rule without scanning the list. We do not use
 *	queue(3) macros for portability and readability.
 */

struct ip_fw {
	u_int32_t version;              /* Version of this structure. MUST be set */
	/* by clients. Should always be */
	/* set to IP_FW_CURRENT_API_VERSION. */
	void *context;                  /* Context that is usable by user processes to */
	/* identify this rule. */
	struct ip_fw    *next;          /* linked list of rules		*/
	struct ip_fw    *next_rule;     /* ptr to next [skipto] rule	*/
	/* 'next_rule' is used to pass up 'set_disable' status		*/

	u_int16_t       act_ofs;        /* offset of action in 32-bit units */
	u_int16_t       cmd_len;        /* # of 32-bit words in cmd	*/
	u_int16_t       rulenum;        /* rule number			*/
	u_int8_t        set;            /* rule set (0..31)		*/
	u_int32_t       set_masks[2];   /* masks for manipulating sets atomically */
#define RESVD_SET       31      /* set for default and persistent rules */
	u_int8_t        _pad;           /* padding			*/

	/* These fields are present in all rules.			*/
	u_int64_t       pcnt;           /* Packet counter		*/
	u_int64_t       bcnt;           /* Byte counter			*/
	u_int32_t       timestamp;      /* tv_sec of last match		*/

	u_int32_t       reserved_1;     /* reserved - set to 0 */
	u_int32_t       reserved_2;     /* reserved - set to 0 */

	ipfw_insn       cmd[1];         /* storage for commands		*/
};

#define ACTION_PTR(rule)                                \
	(ipfw_insn *)( (u_int32_t *)((rule)->cmd) + ((rule)->act_ofs) )

#define RULESIZE(rule)  (sizeof(struct ip_fw) + \
	((struct ip_fw *)(rule))->cmd_len * 4 - 4)

/*
 * This structure is used as a flow mask and a flow id for various
 * parts of the code.
 */
struct ipfw_flow_id {
	u_int32_t       dst_ip;
	u_int32_t       src_ip;
	u_int16_t       dst_port;
	u_int16_t       src_port;
	u_int8_t        proto;
	u_int8_t        flags;  /* protocol-specific flags */
};

/*
 * Dynamic ipfw rule.
 */
typedef struct _ipfw_dyn_rule ipfw_dyn_rule;

#ifdef XNU_KERNEL_PRIVATE

#include <netinet/ip_flowid.h>

/*
 * Note:
 * The internal version of "struct _ipfw_dyn_rule" differs from
 * its external version because the field "id" is of type
 * "struct ip_flow_id" in the internal version. The type of the
 * field "id" for the external version is "ipfw_dyn_rule for
 * backwards compatibility reasons.
 */

struct _ipfw_dyn_rule {
	ipfw_dyn_rule   *next;          /* linked list of rules.	*/
	struct ip_fw *rule;             /* pointer to rule		*/
	/* 'rule' is used to pass up the rule number (from the parent)	*/

	ipfw_dyn_rule *parent;          /* pointer to parent rule	*/
	u_int64_t       pcnt;           /* packet match counter		*/
	u_int64_t       bcnt;           /* byte match counter		*/
	struct ip_flow_id id;           /* (masked) flow id		*/
	u_int32_t       expire;         /* expire time			*/
	u_int32_t       bucket;         /* which bucket in hash table	*/
	u_int32_t       state;          /* state of this rule (typically a
	                                 * combination of TCP flags)
	                                 */
	u_int32_t       ack_fwd;        /* most recent ACKs in forward	*/
	u_int32_t       ack_rev;        /* and reverse directions (used	*/
	                                /* to generate keepalives)	*/
	u_int16_t       dyn_type;       /* rule type			*/
	u_int16_t       count;          /* refcount			*/
};
#else /* XNU_KERNEL_PRIVATE */
struct _ipfw_dyn_rule {
	ipfw_dyn_rule   *next;          /* linked list of rules.	*/
	struct ip_fw *rule;             /* pointer to rule		*/
	/* 'rule' is used to pass up the rule number (from the parent)	*/

	ipfw_dyn_rule *parent;          /* pointer to parent rule	*/
	u_int64_t       pcnt;           /* packet match counter		*/
	u_int64_t       bcnt;           /* byte match counter		*/
	struct ipfw_flow_id id;         /* (masked) flow id		*/
	u_int32_t       expire;         /* expire time			*/
	u_int32_t       bucket;         /* which bucket in hash table	*/
	u_int32_t       state;          /* state of this rule (typically a
	                                 * combination of TCP flags)
	                                 */
	u_int32_t       ack_fwd;        /* most recent ACKs in forward	*/
	u_int32_t       ack_rev;        /* and reverse directions (used	*/
	                                /* to generate keepalives)	*/
	u_int16_t       dyn_type;       /* rule type			*/
	u_int16_t       count;          /* refcount			*/
};
#endif /* XNU_KERNEL_PRIVATE */

/*
 * Definitions for IP option names.
 */
#define IP_FW_IPOPT_LSRR        0x01
#define IP_FW_IPOPT_SSRR        0x02
#define IP_FW_IPOPT_RR          0x04
#define IP_FW_IPOPT_TS          0x08

/*
 * Definitions for TCP option names.
 */
#define IP_FW_TCPOPT_MSS        0x01
#define IP_FW_TCPOPT_WINDOW     0x02
#define IP_FW_TCPOPT_SACK       0x04
#define IP_FW_TCPOPT_TS         0x08
#define IP_FW_TCPOPT_CC         0x10

#define ICMP_REJECT_RST         0x100   /* fake ICMP code (send a TCP RST) */

/*
 * Main firewall chains definitions and global var's definitions.
 */
#ifdef BSD_KERNEL_PRIVATE

#pragma pack(4)
struct ip_fw_32 {
	u_int32_t version;              /* Version of this structure. MUST be set */
	/* by clients. Should always be */
	/* set to IP_FW_CURRENT_API_VERSION. */
	user32_addr_t   context;                        /* Context that is usable by user processes to */
	/* identify this rule. */
	user32_addr_t   next;                   /* linked list of rules         */
	user32_addr_t   next_rule;/* ptr to next [skipto] rule    */
	/* 'next_rule' is used to pass up 'set_disable' status          */

	u_int16_t       act_ofs;                /* offset of action in 32-bit units */
	u_int16_t       cmd_len;        /* # of 32-bit words in cmd     */
	u_int16_t       rulenum;        /* rule number                  */
	u_int8_t        set;            /* rule set (0..31)             */
	u_int32_t       set_masks[2];   /* masks for manipulating sets atomically */
#define RESVD_SET       31      /* set for default and persistent rules */
	u_int8_t        _pad;           /* padding                      */

	/* These fields are present in all rules.                       */
	u_int64_t       pcnt;           /* Packet counter               */
	u_int64_t       bcnt;           /* Byte counter                 */
	u_int32_t       timestamp;      /* tv_sec of last match         */

	u_int32_t       reserved_1;     /* reserved - set to 0 */
	u_int32_t       reserved_2;     /* reserved - set to 0 */

	ipfw_insn       cmd[1];         /* storage for commands         */
};

#pragma pack()

struct ip_fw_64 {
	u_int32_t version;              /* Version of this structure. MUST be set */
	/* by clients. Should always be */
	/* set to IP_FW_CURRENT_API_VERSION. */
	__uint64_t context __attribute__((aligned(8)));         /* Context that is usable by user processes to */
	/* identify this rule. */
	user64_addr_t   next;          /* linked list of rules         */
	user64_addr_t   next_rule;     /* ptr to next [skipto] rule    */
	/* 'next_rule' is used to pass up 'set_disable' status          */

	u_int16_t       act_ofs;        /* offset of action in 32-bit units */
	u_int16_t       cmd_len;        /* # of 32-bit words in cmd     */
	u_int16_t       rulenum;        /* rule number                  */
	u_int8_t        set;            /* rule set (0..31)             */
	u_int32_t       set_masks[2];   /* masks for manipulating sets atomically */
#define RESVD_SET       31      /* set for default and persistent rules */
	u_int8_t        _pad;           /* padding                      */

	/* These fields are present in all rules.                       */
	u_int64_t       pcnt __attribute__((aligned(8)));       /* Packet counter               */
	u_int64_t       bcnt __attribute__((aligned(8)));      /* Byte counter                 */
	u_int32_t       timestamp;      /* tv_sec of last match         */

	u_int32_t       reserved_1;     /* reserved - set to 0 */
	u_int32_t       reserved_2;     /* reserved - set to 0 */

	ipfw_insn       cmd[1];         /* storage for commands         */
};


typedef struct _ipfw_dyn_rule_64 ipfw_dyn_rule_64;
typedef struct _ipfw_dyn_rule_32 ipfw_dyn_rule_32;

#pragma pack(4)
struct _ipfw_dyn_rule_32 {
	user32_addr_t   next;                   /* linked list of rules.        */
	user32_addr_t   rule;                   /* pointer to rule              */
	/* 'rule' is used to pass up the rule number (from the parent)  */

	user32_addr_t   parent;          /* pointer to parent rule       */
	u_int64_t       pcnt;           /* packet match counter         */
	u_int64_t       bcnt;           /* byte match counter           */
	struct ipfw_flow_id id;         /* (masked) flow id             */
	u_int32_t       expire;         /* expire time                  */
	u_int32_t       bucket;         /* which bucket in hash table   */
	u_int32_t       state;          /* state of this rule (typically a
	                                 * combination of TCP flags)
	                                 */
	u_int32_t       ack_fwd;        /* most recent ACKs in forward  */
	u_int32_t       ack_rev;        /* and reverse directions (used */
	                                /* to generate keepalives)      */
	u_int16_t       dyn_type;       /* rule type                    */
	u_int16_t       count;          /* refcount                     */
};

#pragma pack()

struct _ipfw_dyn_rule_64 {
	user64_addr_t   next;          /* linked list of rules.        */
	user64_addr_t   rule;             /* pointer to rule              */
	/* 'rule' is used to pass up the rule number (from the parent)  */

	user64_addr_t   parent;          /* pointer to parent rule       */
	u_int64_t       pcnt;           /* packet match counter         */
	u_int64_t       bcnt;           /* byte match counter           */
	struct ipfw_flow_id id;         /* (masked) flow id             */
	u_int32_t       expire;         /* expire time                  */
	u_int32_t       bucket;         /* which bucket in hash table   */
	u_int32_t       state;          /* state of this rule (typically a
	                                 * combination of TCP flags)
	                                 */
	u_int32_t       ack_fwd;        /* most recent ACKs in forward  */
	u_int32_t       ack_rev;        /* and reverse directions (used */
	                                /* to generate keepalives)      */
	u_int16_t       dyn_type;       /* rule type                    */
	u_int16_t       count;          /* refcount                     */
};


typedef struct  _ipfw_insn_pipe_64 {
	ipfw_insn       o;
	user64_addr_t   pipe_ptr;      /* XXX */
} ipfw_insn_pipe_64;

typedef struct  _ipfw_insn_pipe_32 {
	ipfw_insn       o;
	user32_addr_t   pipe_ptr;      /* XXX */
} ipfw_insn_pipe_32;


#define IPFW_DEFAULT_RULE       65535

#if IPFIREWALL

#define IP_FW_PORT_DYNT_FLAG    0x10000
#define IP_FW_PORT_TEE_FLAG     0x20000
#define IP_FW_PORT_DENY_FLAG    0x40000

#ifdef PRIVATE
#include <netinet/ip_flowid.h>
#else
struct ip_fw_args;
#endif
/*
 * Function definitions.
 */

/* Firewall hooks */
struct sockopt;
struct dn_flow_set;

void flush_pipe_ptrs(struct dn_flow_set *match); /* used by dummynet */
void ipfw_init(void);   /* called from raw_ip.c: load_ipfw() */

typedef int ip_fw_chk_t (struct ip_fw_args *args);
typedef int ip_fw_ctl_t (struct sockopt *);
extern ip_fw_chk_t *ip_fw_chk_ptr;
extern ip_fw_ctl_t *ip_fw_ctl_ptr;
extern int fw_one_pass;
extern int fw_enable;
#define IPFW_LOADED     (ip_fw_chk_ptr != NULL)
#endif /* IPFIREWALL */
#endif /* BSD_KERNEL_PRIVATE */

#endif /* __APPLE_API_OBSOLETE */
#endif /* _IPFW2_H */
