/*
 * Copyright (c) 2000-2011 Apple Computer, Inc. All rights reserved.
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
/*	$KAME: pfkeyv2.h,v 1.10 2000/03/22 07:04:20 sakane Exp $	*/

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * This file has been derived rfc 2367,
 * And added some flags of SADB_KEY_FLAGS_ as SADB_X_EXT_.
 *	sakane@ydc.co.jp
 */

#ifndef _NET_PFKEYV2_H_
#define _NET_PFKEYV2_H_
#include <sys/appleapiopts.h>
#include <net/if.h>

/*
 *  This file defines structures and symbols for the PF_KEY Version 2
 *  key management interface. It was written at the U.S. Naval Research
 *  Laboratory. This file is in the public domain. The authors ask that
 *  you leave this credit intact on any copies of this file.
 */
#ifndef __PFKEY_V2_H
#define __PFKEY_V2_H 1

#define PF_KEY_V2 2
#define PFKEYV2_REVISION        199806L

#define SADB_RESERVED    0
#define SADB_GETSPI      1
#define SADB_UPDATE      2
#define SADB_ADD         3
#define SADB_DELETE      4
#define SADB_GET         5
#define SADB_ACQUIRE     6
#define SADB_REGISTER    7
#define SADB_EXPIRE      8
#define SADB_FLUSH       9
#define SADB_DUMP        10
#define SADB_X_PROMISC   11
#define SADB_X_PCHANGE   12

#define SADB_X_SPDUPDATE  13
#define SADB_X_SPDADD     14
#define SADB_X_SPDDELETE  15    /* by policy index */
#define SADB_X_SPDGET     16
#define SADB_X_SPDACQUIRE 17
#define SADB_X_SPDDUMP    18
#define SADB_X_SPDFLUSH   19
#define SADB_X_SPDSETIDX  20
#define SADB_X_SPDEXPIRE  21
#define SADB_X_SPDDELETE2 22    /* by policy id */
#define SADB_GETSASTAT    23
#define SADB_X_SPDENABLE  24    /* by policy id */
#define SADB_X_SPDDISABLE 25    /* by policy id */
#define SADB_MIGRATE      26
#define SADB_MAX          26

struct sadb_msg {
	u_int8_t sadb_msg_version;
	u_int8_t sadb_msg_type;
	u_int8_t sadb_msg_errno;
	u_int8_t sadb_msg_satype;
	u_int16_t sadb_msg_len;
	u_int16_t sadb_msg_reserved;
	u_int32_t sadb_msg_seq;
	u_int32_t sadb_msg_pid;
};

struct sadb_ext {
	u_int16_t sadb_ext_len;
	u_int16_t sadb_ext_type;
};

struct sadb_sa {
	u_int16_t sadb_sa_len;
	u_int16_t sadb_sa_exttype;
	u_int32_t sadb_sa_spi;
	u_int8_t sadb_sa_replay;
	u_int8_t sadb_sa_state;
	u_int8_t sadb_sa_auth;
	u_int8_t sadb_sa_encrypt;
	u_int32_t sadb_sa_flags;
};

#ifdef PRIVATE
struct sadb_sa_2 {
	struct sadb_sa  sa;
	u_int16_t               sadb_sa_natt_port;
	union {
		u_int16_t               sadb_reserved0;
		u_int16_t               sadb_sa_natt_interval;
	};

	u_int16_t               sadb_sa_natt_offload_interval;
#define SADB_SA_NATT_SRC_PORT   1
	u_int16_t               sadb_sa_natt_src_port;
};
#endif /* PRIVATE */

struct sadb_lifetime {
	u_int16_t sadb_lifetime_len;
	u_int16_t sadb_lifetime_exttype;
	u_int32_t sadb_lifetime_allocations;
	u_int64_t sadb_lifetime_bytes;
	u_int64_t sadb_lifetime_addtime;
	u_int64_t sadb_lifetime_usetime;
};

struct sadb_address {
	u_int16_t sadb_address_len;
	u_int16_t sadb_address_exttype;
	u_int8_t sadb_address_proto;
	u_int8_t sadb_address_prefixlen;
	u_int16_t sadb_address_reserved;
};

struct sadb_key {
	u_int16_t sadb_key_len;
	u_int16_t sadb_key_exttype;
	u_int16_t sadb_key_bits;
	u_int16_t sadb_key_reserved;
};

struct sadb_ident {
	u_int16_t sadb_ident_len;
	u_int16_t sadb_ident_exttype;
	u_int16_t sadb_ident_type;
	u_int16_t sadb_ident_reserved;
	u_int64_t sadb_ident_id;
};

struct sadb_sens {
	u_int16_t sadb_sens_len;
	u_int16_t sadb_sens_exttype;
	u_int32_t sadb_sens_dpd;
	u_int8_t sadb_sens_sens_level;
	u_int8_t sadb_sens_sens_len;
	u_int8_t sadb_sens_integ_level;
	u_int8_t sadb_sens_integ_len;
	u_int32_t sadb_sens_reserved;
};

struct sadb_prop {
	u_int16_t sadb_prop_len;
	u_int16_t sadb_prop_exttype;
	u_int8_t sadb_prop_replay;
	u_int8_t sadb_prop_reserved[3];
};

struct sadb_comb {
	u_int8_t sadb_comb_auth;
	u_int8_t sadb_comb_encrypt;
	u_int16_t sadb_comb_flags;
	u_int16_t sadb_comb_auth_minbits;
	u_int16_t sadb_comb_auth_maxbits;
	u_int16_t sadb_comb_encrypt_minbits;
	u_int16_t sadb_comb_encrypt_maxbits;
	u_int32_t sadb_comb_reserved;
	u_int32_t sadb_comb_soft_allocations;
	u_int32_t sadb_comb_hard_allocations;
	u_int64_t sadb_comb_soft_bytes;
	u_int64_t sadb_comb_hard_bytes;
	u_int64_t sadb_comb_soft_addtime;
	u_int64_t sadb_comb_hard_addtime;
	u_int64_t sadb_comb_soft_usetime;
	u_int64_t sadb_comb_hard_usetime;
};

struct sadb_supported {
	u_int16_t sadb_supported_len;
	u_int16_t sadb_supported_exttype;
	u_int32_t sadb_supported_reserved;
};

struct sadb_alg {
	u_int8_t sadb_alg_id;
	u_int8_t sadb_alg_ivlen;
	u_int16_t sadb_alg_minbits;
	u_int16_t sadb_alg_maxbits;
	u_int16_t sadb_alg_reserved;
};

struct sadb_spirange {
	u_int16_t sadb_spirange_len;
	u_int16_t sadb_spirange_exttype;
	u_int32_t sadb_spirange_min;
	u_int32_t sadb_spirange_max;
	u_int32_t sadb_spirange_reserved;
};

struct sadb_x_kmprivate {
	u_int16_t sadb_x_kmprivate_len;
	u_int16_t sadb_x_kmprivate_exttype;
	u_int32_t sadb_x_kmprivate_reserved;
};

/*
 * XXX Additional SA Extension.
 * mode: tunnel or transport
 * reqid: to make SA unique nevertheless the address pair of SA are same.
 *        Mainly it's for VPN.
 */
struct sadb_x_sa2 {
	u_int16_t sadb_x_sa2_len;
	u_int16_t sadb_x_sa2_exttype;
	u_int8_t sadb_x_sa2_mode;
	union {
		u_int8_t sadb_x_sa2_reserved1;
#ifdef PRIVATE
		u_int8_t sadb_x_sa2_alwaysexpire;
#endif
	};
	union {
		u_int16_t sadb_x_sa2_reserved2;
#ifdef PRIVATE
		u_int16_t sadb_x_sa2_flags;
#endif
	};
	u_int32_t sadb_x_sa2_sequence;
	u_int32_t sadb_x_sa2_reqid;
};

/* XXX Policy Extension */
/* sizeof(struct sadb_x_policy) == 16 */
struct sadb_x_policy {
	u_int16_t sadb_x_policy_len;
	u_int16_t sadb_x_policy_exttype;
	u_int16_t sadb_x_policy_type;   /* See policy type of ipsec.h */
	u_int8_t sadb_x_policy_dir;     /* direction, see ipsec.h */
	u_int8_t sadb_x_policy_reserved;
	u_int32_t sadb_x_policy_id;
	u_int32_t sadb_x_policy_reserved2;
};
/*
 * When policy_type == IPSEC, it is followed by some of
 * the ipsec policy request.
 * [total length of ipsec policy requests]
 *	= (sadb_x_policy_len * sizeof(uint64_t) - sizeof(struct sadb_x_policy))
 */
#ifdef PRIVATE
/* IPsec Interface Extension:
 * IPsec interface can be specified alone, or all three
 * of internal, outgoing, and IPsec interfaces must be
 * specified.
 */
struct sadb_x_ipsecif {
	u_int16_t sadb_x_ipsecif_len;
	u_int16_t sadb_x_ipsecif_exttype;
	char sadb_x_ipsecif_internal_if[IFXNAMSIZ]; /* Steal packets from this interface */
	char sadb_x_ipsecif_outgoing_if[IFXNAMSIZ]; /* Send packets out on this interface */
	char sadb_x_ipsecif_ipsec_if[IFXNAMSIZ];  /* Direct packets through ipsec interface */
	u_int16_t sadb_x_ipsecif_init_disabled; /* 0 or 1, flag to ignore policy */
	u_int16_t reserved;
};
#endif
/* XXX IPsec Policy Request Extension */
/*
 * This structure is aligned 8 bytes.
 */
struct sadb_x_ipsecrequest {
	u_int16_t sadb_x_ipsecrequest_len; /* structure length aligned to 8 bytes.
	                                    * This value is true length of bytes.
	                                    * Not in units of 64 bits. */
	u_int16_t sadb_x_ipsecrequest_proto; /* See ipsec.h */
	u_int8_t sadb_x_ipsecrequest_mode; /* See IPSEC_MODE_XX in ipsec.h. */
	u_int8_t sadb_x_ipsecrequest_level; /* See IPSEC_LEVEL_XX in ipsec.h */
	u_int16_t sadb_x_ipsecrequest_reqid; /* See ipsec.h */

	/*
	 * followed by source IP address of SA, and immediately followed by
	 * destination IP address of SA.  These encoded into two of sockaddr
	 * structure without any padding.  Must set each sa_len exactly.
	 * Each of length of the sockaddr structure are not aligned to 64bits,
	 * but sum of x_request and addresses is aligned to 64bits.
	 */
};

struct sadb_session_id {
	u_int16_t            sadb_session_id_len;
	u_int16_t            sadb_session_id_exttype;
	/* [0] is an arbitrary handle that means something only for requester
	 * [1] is a global session id for lookups in the kernel and racoon.
	 */
	u_int64_t            sadb_session_id_v[2];
} __attribute__ ((aligned(8)));

struct sastat {
	u_int32_t            spi;               /* SPI Value, network byte order */
	u_int32_t            created;           /* for lifetime */
	struct sadb_lifetime lft_c;             /* CURRENT lifetime. */
}; // no need to align

struct sadb_sastat {
	u_int16_t            sadb_sastat_len;
	u_int16_t            sadb_sastat_exttype;
	u_int32_t            sadb_sastat_dir;
	u_int32_t            sadb_sastat_reserved;
	u_int32_t            sadb_sastat_list_len;
	/* list of struct sastat comes after */
} __attribute__ ((aligned(8)));

#define SADB_EXT_RESERVED             0
#define SADB_EXT_SA                   1
#define SADB_EXT_LIFETIME_CURRENT     2
#define SADB_EXT_LIFETIME_HARD        3
#define SADB_EXT_LIFETIME_SOFT        4
#define SADB_EXT_ADDRESS_SRC          5
#define SADB_EXT_ADDRESS_DST          6
#define SADB_EXT_ADDRESS_PROXY        7
#define SADB_EXT_KEY_AUTH             8
#define SADB_EXT_KEY_ENCRYPT          9
#define SADB_EXT_IDENTITY_SRC         10
#define SADB_EXT_IDENTITY_DST         11
#define SADB_EXT_SENSITIVITY          12
#define SADB_EXT_PROPOSAL             13
#define SADB_EXT_SUPPORTED_AUTH       14
#define SADB_EXT_SUPPORTED_ENCRYPT    15
#define SADB_EXT_SPIRANGE             16
#define SADB_X_EXT_KMPRIVATE          17
#define SADB_X_EXT_POLICY             18
#define SADB_X_EXT_SA2                19
#define SADB_EXT_SESSION_ID           20
#define SADB_EXT_SASTAT               21
#define SADB_X_EXT_IPSECIF            22
#define SADB_X_EXT_ADDR_RANGE_SRC_START 23
#define SADB_X_EXT_ADDR_RANGE_SRC_END   24
#define SADB_X_EXT_ADDR_RANGE_DST_START 25
#define SADB_X_EXT_ADDR_RANGE_DST_END   26
#define SADB_EXT_MIGRATE_ADDRESS_SRC  27
#define SADB_EXT_MIGRATE_ADDRESS_DST  28
#define SADB_X_EXT_MIGRATE_IPSECIF    29
#define SADB_EXT_MAX                  29

#define SADB_SATYPE_UNSPEC      0
#define SADB_SATYPE_AH          2
#define SADB_SATYPE_ESP         3
#define SADB_SATYPE_RSVP        5
#define SADB_SATYPE_OSPFV2      6
#define SADB_SATYPE_RIPV2       7
#define SADB_SATYPE_MIP         8
#define SADB_X_SATYPE_IPCOMP    9
#define SADB_X_SATYPE_POLICY    10
#define SADB_SATYPE_MAX         11

#define SADB_SASTATE_LARVAL   0
#define SADB_SASTATE_MATURE   1
#define SADB_SASTATE_DYING    2
#define SADB_SASTATE_DEAD     3
#define SADB_SASTATE_MAX      3

#define SADB_SAFLAGS_PFS      1

/* RFC2367 numbers - meets RFC2407 */
#define SADB_AALG_NONE          0
#define SADB_AALG_MD5HMAC       1       /*2*/
#define SADB_AALG_SHA1HMAC      2       /*3*/
#define SADB_AALG_MAX           8
/* private allocations - based on RFC2407/IANA assignment */
#define SADB_X_AALG_SHA2_256    6       /*5*/
#define SADB_X_AALG_SHA2_384    7       /*6*/
#define SADB_X_AALG_SHA2_512    8       /*7*/
/* private allocations should use 249-255 (RFC2407) */
#define SADB_X_AALG_MD5         3       /*249*/	/* Keyed MD5 */
#define SADB_X_AALG_SHA         4       /*250*/	/* Keyed SHA */
#define SADB_X_AALG_NULL        5       /*251*/	/* null authentication */

/* RFC2367 numbers - meets RFC2407 */
#define SADB_EALG_NONE          0
#define SADB_EALG_DESCBC        1       /*2*/
#define SADB_EALG_3DESCBC       2       /*3*/
#define SADB_EALG_NULL          3       /*11*/
#define SADB_EALG_MAX           12
/* private allocations - based on RFC2407/IANA assignment */
#define SADB_X_EALG_CAST128CBC  5       /*6*/
#define SADB_X_EALG_BLOWFISHCBC 4       /*7*/
#define SADB_X_EALG_RIJNDAELCBC 12
#define SADB_X_EALG_AESCBC      12
#define SADB_X_EALG_AES         12
#define SADB_X_EALG_AES_GCM     13
#define SADB_X_EALG_CHACHA20POLY1305 14
/* private allocations should use 249-255 (RFC2407) */

#if 1   /*nonstandard */
#define SADB_X_CALG_NONE        0
#define SADB_X_CALG_OUI         1
#define SADB_X_CALG_DEFLATE     2
#define SADB_X_CALG_LZS         3
#define SADB_X_CALG_MAX         4
#endif

#define SADB_IDENTTYPE_RESERVED   0
#define SADB_IDENTTYPE_PREFIX     1
#define SADB_IDENTTYPE_FQDN       2
#define SADB_IDENTTYPE_USERFQDN   3
#define SADB_X_IDENTTYPE_ADDR     4
#define SADB_IDENTTYPE_MAX        4

/* `flags' in sadb_sa structure holds followings */
#define SADB_X_EXT_NONE         0x0000  /* i.e. new format. */
#define SADB_X_EXT_OLD          0x0001  /* old format. */
#ifdef PRIVATE
#define SADB_X_EXT_NATT                         0x0002  /* Use UDP encapsulation to traverse NAT */
#define SADB_X_EXT_NATT_KEEPALIVE       0x0004  /* Local node is behind NAT, send keepalives */
/* Should only be set for outbound SAs */
#define SADB_X_EXT_NATT_MULTIPLEUSERS 0x0008    /* For use on VPN server - support multiple users       */

#endif /* PRIVATE */

#define SADB_X_EXT_IV4B         0x0010  /* IV length of 4 bytes in use */
#define SADB_X_EXT_DERIV        0x0020  /* DES derived */
#define SADB_X_EXT_CYCSEQ       0x0040  /* allowing to cyclic sequence. */

/* three of followings are exclusive flags each them */
#define SADB_X_EXT_PSEQ         0x0000  /* sequencial padding for ESP */
#define SADB_X_EXT_PRAND        0x0100  /* random padding for ESP */
#define SADB_X_EXT_PZERO        0x0200  /* zero padding for ESP */
#define SADB_X_EXT_PMASK        0x0300  /* mask for padding flag */

#define SADB_X_EXT_IIV          0x0400 /* Implicit IV */

#ifdef PRIVATE
#define SADB_X_EXT_NATT_DETECTED_PEER 0x1000
#define SADB_X_EXT_ESP_KEEPALIVE      0x2000
#define SADB_X_EXT_PUNT_RX_KEEPALIVE  0x4000
#define SADB_X_EXT_NATT_KEEPALIVE_OFFLOAD  0x8000
#endif /* PRIVATE */

#ifdef PRIVATE
#define NATT_KEEPALIVE_OFFLOAD_INTERVAL 0x1
#endif

#if 1
#define SADB_X_EXT_RAWCPI       0x0080  /* use well known CPI (IPComp) */
#endif

#define SADB_KEY_FLAGS_MAX      0x7fff

#ifdef PRIVATE
#define SADB_X_EXT_SA2_DELETE_ON_DETACH   0x0001
#define SADB_X_EXT_SA2_SEQ_PER_TRAFFIC_CLASS  0x0002
#endif

/* SPI size for PF_KEYv2 */
#define PFKEY_SPI_SIZE  sizeof(u_int32_t)

/* Identifier for menber of lifetime structure */
#define SADB_X_LIFETIME_ALLOCATIONS     0
#define SADB_X_LIFETIME_BYTES           1
#define SADB_X_LIFETIME_ADDTIME         2
#define SADB_X_LIFETIME_USETIME         3

/* The rate for SOFT lifetime against HARD one. */
#define PFKEY_SOFT_LIFETIME_RATE        80

/* Utilities */
#define PFKEY_ALIGN8(a) (1 + (((a) - 1) | (8 - 1)))
#define PFKEY_EXTLEN(msg) \
	PFKEY_UNUNIT64(((struct sadb_ext *)(msg))->sadb_ext_len)
#define PFKEY_ADDR_PREFIX(ext) \
	(((struct sadb_address *)(ext))->sadb_address_prefixlen)
#define PFKEY_ADDR_PROTO(ext) \
	(((struct sadb_address *)(ext))->sadb_address_proto)
#define PFKEY_ADDR_SADDR(ext) \
	((struct sockaddr *)((caddr_t)(ext) + sizeof(struct sadb_address)))

/* in 64bits */
#define PFKEY_UNUNIT64(a)       ((a) << 3)
#define PFKEY_UNIT64(a)         ((a) >> 3)

#endif /* __PFKEY_V2_H */

#endif /* _NET_PFKEYV2_H_ */
