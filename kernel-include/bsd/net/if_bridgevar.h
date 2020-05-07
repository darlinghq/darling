/*
 * Copyright (c) 2004-2014 Apple Inc. All rights reserved.
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
 * Copyright 2001 Wasabi Systems, Inc.
 * All rights reserved.
 *
 * Written by Jason R. Thorpe for Wasabi Systems, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed for the NetBSD Project by
 *	Wasabi Systems, Inc.
 * 4. The name of Wasabi Systems, Inc. may not be used to endorse
 *    or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY WASABI SYSTEMS, INC. ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL WASABI SYSTEMS, INC
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 1999, 2000 Jason L. Wright (jason@thought.net)
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Jason L. Wright
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * OpenBSD: if_bridge.h,v 1.14 2001/03/22 03:48:29 jason Exp
 *
 * $FreeBSD$
 */

/*
 * Data structure and control definitions for bridge interfaces.
 */

#ifndef _NET_IF_BRIDGEVAR_H_
#define _NET_IF_BRIDGEVAR_H_

#ifdef PRIVATE

#include <sys/queue.h>

#include <net/if.h>
#include <net/ethernet.h>

/*
 * Commands used in the SIOCSDRVSPEC ioctl.  Note the lookup of the
 * bridge interface itself is keyed off the ifdrv structure.
 */
#define BRDGADD                 0       /* add bridge member (ifbreq) */
#define BRDGDEL                 1       /* delete bridge member (ifbreq) */
#define BRDGGIFFLGS             2       /* get member if flags (ifbreq) */
#define BRDGSIFFLGS             3       /* set member if flags (ifbreq) */
#define BRDGSCACHE              4       /* set cache size (ifbrparam) */
#define BRDGGCACHE              5       /* get cache size (ifbrparam) */
#define BRDGGIFS                6       /* get member list (ifbifconf) */
#define BRDGRTS                 7       /* get address list (ifbaconf) */
#define BRDGSADDR               8       /* set static address (ifbareq) */
#define BRDGSTO                 9       /* set cache timeout (ifbrparam) */
#define BRDGGTO                 10      /* get cache timeout (ifbrparam) */
#define BRDGDADDR               11      /* delete address (ifbareq) */
#define BRDGFLUSH               12      /* flush address cache (ifbreq) */

#define BRDGGPRI                13      /* get priority (ifbrparam) */
#define BRDGSPRI                14      /* set priority (ifbrparam) */
#define BRDGGHT                 15      /* get hello time (ifbrparam) */
#define BRDGSHT                 16      /* set hello time (ifbrparam) */
#define BRDGGFD                 17      /* get forward delay (ifbrparam) */
#define BRDGSFD                 18      /* set forward delay (ifbrparam) */
#define BRDGGMA                 19      /* get max age (ifbrparam) */
#define BRDGSMA                 20      /* set max age (ifbrparam) */
#define BRDGSIFPRIO             21      /* set if priority (ifbreq) */
#define BRDGSIFCOST             22      /* set if path cost (ifbreq) */
#define BRDGGFILT               23      /* get filter flags (ifbrparam) */
#define BRDGSFILT               24      /* set filter flags (ifbrparam) */
#define BRDGPURGE               25      /* purge address cache for a particular interface (ifbreq) */
#define BRDGADDS                26      /* add bridge span member (ifbreq) */
#define BRDGDELS                27      /* delete bridge span member (ifbreq) */
#define BRDGPARAM               28      /* get bridge STP params (ifbropreq) */
#define BRDGGRTE                29      /* get cache drops (ifbrparam) */
#define BRDGGIFSSTP             30      /* get member STP params list (ifbpstpconf) */
#define BRDGSPROTO              31      /* set protocol (ifbrparam) */
#define BRDGSTXHC               32      /* set tx hold count (ifbrparam) */
#define BRDGSIFAMAX             33      /* set max interface addrs (ifbreq) */
#define BRDGGHOSTFILTER         34      /* set max interface addrs (ifbrhostfilter) */
#define BRDGSHOSTFILTER         35      /* set max interface addrs (ifbrhostfilter) */

/*
 * Generic bridge control request.
 */
#pragma pack(4)

struct ifbreq {
	char            ifbr_ifsname[IFNAMSIZ]; /* member if name */
	uint32_t        ifbr_ifsflags;          /* member if flags */
	uint32_t        ifbr_stpflags;          /* member if STP flags */
	uint32_t        ifbr_path_cost;         /* member if STP cost */
	uint8_t         ifbr_portno;            /* member if port number */
	uint8_t         ifbr_priority;          /* member if STP priority */
	uint8_t         ifbr_proto;             /* member if STP protocol */
	uint8_t         ifbr_role;              /* member if STP role */
	uint8_t         ifbr_state;             /* member if STP state */
	uint32_t        ifbr_addrcnt;           /* member if addr number */
	uint32_t        ifbr_addrmax;           /* member if addr max */
	uint32_t        ifbr_addrexceeded;      /* member if addr violations */
	uint8_t         pad[32];
};

#pragma pack()

/* BRDGGIFFLAGS, BRDGSIFFLAGS */
#define IFBIF_LEARNING          0x0001  /* if can learn */
#define IFBIF_DISCOVER          0x0002  /* if sends packets w/ unknown dest. */
#define IFBIF_STP               0x0004  /* if participates in spanning tree */
#define IFBIF_SPAN              0x0008  /* if is a span port */
#define IFBIF_STICKY            0x0010  /* if learned addresses stick */
#define IFBIF_BSTP_EDGE         0x0020  /* member stp edge port */
#define IFBIF_BSTP_AUTOEDGE     0x0040  /* member stp autoedge enabled */
#define IFBIF_BSTP_PTP          0x0080  /* member stp point to point */
#define IFBIF_BSTP_AUTOPTP      0x0100  /* member stp autoptp enabled */
#define IFBIF_BSTP_ADMEDGE      0x0200  /* member stp admin edge enabled */
#define IFBIF_BSTP_ADMCOST      0x0400  /* member stp admin path cost */
#define IFBIF_PRIVATE           0x0800  /* if is a private segment */

#define IFBIFBITS       "\020\001LEARNING\002DISCOVER\003STP\004SPAN" \
	                "\005STICKY\014PRIVATE\006EDGE\007AUTOEDGE\010PTP" \
	                "\011AUTOPTP"
#define IFBIFMASK       ~(IFBIF_BSTP_EDGE|IFBIF_BSTP_AUTOEDGE|IFBIF_BSTP_PTP| \
	                IFBIF_BSTP_AUTOPTP|IFBIF_BSTP_ADMEDGE| \
	                IFBIF_BSTP_ADMCOST)     /* not saved */

/* BRDGFLUSH */
#define IFBF_FLUSHDYN           0x00    /* flush learned addresses only */
#define IFBF_FLUSHALL           0x01    /* flush all addresses */

/* BRDGSFILT */
#define IFBF_FILT_USEIPF        0x00000001 /* run pfil hooks on the bridge
	                                    *  interface */
#define IFBF_FILT_MEMBER        0x00000002 /* run pfil hooks on the member
	                                    *  interfaces */
#define IFBF_FILT_ONLYIP        0x00000004 /* only pass IP[46] packets when
	                                    *  pfil is enabled */
#define IFBF_FILT_MASK          0x00000007 /* mask of valid values */


/* APPLE MODIFICATION <jhw@apple.com>: Default is to pass non-IP packets. */
#define IFBF_FILT_DEFAULT       ( IFBF_FILT_USEIPF | IFBF_FILT_MEMBER )
#if 0
#define IFBF_FILT_DEFAULT       (IFBF_FILT_USEIPF | \
IFBF_FILT_MEMBER | \
IFBF_FILT_ONLYIP)
#endif

/*
 * Interface list structure.
 */

#pragma pack(4)

#ifndef XNU_KERNEL_PRIVATE

struct ifbifconf {
	uint32_t        ifbic_len;      /* buffer size */
	union {
		caddr_t ifbicu_buf;
		struct ifbreq *ifbicu_req;
#define ifbic_buf       ifbic_ifbicu.ifbicu_buf
#define ifbic_req       ifbic_ifbicu.ifbicu_req
	} ifbic_ifbicu;
};

#else /* XNU_KERNEL_PRIVATE */

struct ifbifconf32 {
	uint32_t        ifbic_len;      /* buffer size */
	union {
		user32_addr_t   ifbicu_buf;
		user32_addr_t   ifbicu_req;
#define ifbic_buf       ifbic_ifbicu.ifbicu_buf
#define ifbic_req       ifbic_ifbicu.ifbicu_req
	} ifbic_ifbicu;
};

struct ifbifconf64 {
	uint32_t        ifbic_len;      /* buffer size */
	union {
		user64_addr_t   ifbicu_buf;
		user64_addr_t   ifbicu_req;
	} ifbic_ifbicu;
};
#endif /* XNU_KERNEL_PRIVATE */

#pragma pack()

/*
 * Bridge address request.
 */

#pragma pack(4)

#ifndef XNU_KERNEL_PRIVATE

struct ifbareq {
	char            ifba_ifsname[IFNAMSIZ]; /* member if name */
	unsigned long   ifba_expire;            /* address expire time */
	uint8_t         ifba_flags;             /* address flags */
	uint8_t         ifba_dst[ETHER_ADDR_LEN];/* destination address */
	uint16_t        ifba_vlan;              /* vlan id */
};

#else /* XNU_KERNEL_PRIVATE */

struct ifbareq32 {
	char            ifba_ifsname[IFNAMSIZ]; /* member if name */
	uint32_t        ifba_expire;            /* address expire time */
	uint8_t         ifba_flags;             /* address flags */
	uint8_t         ifba_dst[ETHER_ADDR_LEN];/* destination address */
	uint16_t        ifba_vlan;              /* vlan id */
};

struct ifbareq64 {
	char            ifba_ifsname[IFNAMSIZ]; /* member if name */
	uint64_t        ifba_expire;            /* address expire time */
	uint8_t         ifba_flags;             /* address flags */
	uint8_t         ifba_dst[ETHER_ADDR_LEN];/* destination address */
	uint16_t        ifba_vlan;              /* vlan id */
};
#endif /* XNU_KERNEL_PRIVATE */

#pragma pack()

#define IFBAF_TYPEMASK  0x03    /* address type mask */
#define IFBAF_DYNAMIC   0x00    /* dynamically learned address */
#define IFBAF_STATIC    0x01    /* static address */
#define IFBAF_STICKY    0x02    /* sticky address */

#define IFBAFBITS       "\020\1STATIC\2STICKY"

/*
 * Address list structure.
 */

#pragma pack(4)

#ifndef XNU_KERNEL_PRIVATE

struct ifbaconf {
	uint32_t        ifbac_len;      /* buffer size */
	union {
		caddr_t ifbacu_buf;
		struct ifbareq *ifbacu_req;
#define ifbac_buf       ifbac_ifbacu.ifbacu_buf
#define ifbac_req       ifbac_ifbacu.ifbacu_req
	} ifbac_ifbacu;
};

#else /* XNU_KERNEL_PRIVATE */

struct ifbaconf32 {
	uint32_t        ifbac_len;      /* buffer size */
	union {
		user32_addr_t   ifbacu_buf;
		user32_addr_t   ifbacu_req;
#define ifbac_buf       ifbac_ifbacu.ifbacu_buf
#define ifbac_req       ifbac_ifbacu.ifbacu_req
	} ifbac_ifbacu;
};

struct ifbaconf64 {
	uint32_t        ifbac_len;      /* buffer size */
	union {
		user64_addr_t   ifbacu_buf;
		user64_addr_t   ifbacu_req;
	} ifbac_ifbacu;
};
#endif /* XNU_KERNEL_PRIVATE */

#pragma pack()

/*
 * Bridge parameter structure.
 */

#pragma pack(4)

struct ifbrparam {
	union {
		uint32_t ifbrpu_int32;
		uint16_t ifbrpu_int16;
		uint8_t ifbrpu_int8;
	} ifbrp_ifbrpu;
};

#pragma pack()

#define ifbrp_csize     ifbrp_ifbrpu.ifbrpu_int32       /* cache size */
#define ifbrp_ctime     ifbrp_ifbrpu.ifbrpu_int32       /* cache time (sec) */
#define ifbrp_prio      ifbrp_ifbrpu.ifbrpu_int16       /* bridge priority */
#define ifbrp_proto     ifbrp_ifbrpu.ifbrpu_int8        /* bridge protocol */
#define ifbrp_txhc      ifbrp_ifbrpu.ifbrpu_int8        /* bpdu tx holdcount */
#define ifbrp_hellotime ifbrp_ifbrpu.ifbrpu_int8        /* hello time (sec) */
#define ifbrp_fwddelay  ifbrp_ifbrpu.ifbrpu_int8        /* fwd time (sec) */
#define ifbrp_maxage    ifbrp_ifbrpu.ifbrpu_int8        /* max age (sec) */
#define ifbrp_cexceeded ifbrp_ifbrpu.ifbrpu_int32       /* # of cache dropped
	                                                 * adresses */
#define ifbrp_filter    ifbrp_ifbrpu.ifbrpu_int32       /* filtering flags */

/*
 * Bridge current operational parameters structure.
 */

#pragma pack(4)

#ifndef XNU_KERNEL_PRIVATE

struct ifbropreq {
	uint8_t         ifbop_holdcount;
	uint8_t         ifbop_maxage;
	uint8_t         ifbop_hellotime;
	uint8_t         ifbop_fwddelay;
	uint8_t         ifbop_protocol;
	uint16_t        ifbop_priority;
	uint16_t        ifbop_root_port;
	uint32_t        ifbop_root_path_cost;
	uint64_t        ifbop_bridgeid;
	uint64_t        ifbop_designated_root;
	uint64_t        ifbop_designated_bridge;
	struct timeval  ifbop_last_tc_time;
};

#else /* XNU_KERNEL_PRIVATE */

struct ifbropreq32 {
	uint8_t         ifbop_holdcount;
	uint8_t         ifbop_maxage;
	uint8_t         ifbop_hellotime;
	uint8_t         ifbop_fwddelay;
	uint8_t         ifbop_protocol;
	uint16_t        ifbop_priority;
	uint16_t        ifbop_root_port;
	uint32_t        ifbop_root_path_cost;
	uint64_t        ifbop_bridgeid;
	uint64_t        ifbop_designated_root;
	uint64_t        ifbop_designated_bridge;
	struct timeval  ifbop_last_tc_time;
};

struct ifbropreq64 {
	uint8_t         ifbop_holdcount;
	uint8_t         ifbop_maxage;
	uint8_t         ifbop_hellotime;
	uint8_t         ifbop_fwddelay;
	uint8_t         ifbop_protocol;
	uint16_t        ifbop_priority;
	uint16_t        ifbop_root_port;
	uint32_t        ifbop_root_path_cost;
	uint64_t        ifbop_bridgeid;
	uint64_t        ifbop_designated_root;
	uint64_t        ifbop_designated_bridge;
	struct timeval  ifbop_last_tc_time;
};

#endif

#pragma pack()

/*
 * Bridge member operational STP params structure.
 */

#pragma pack(4)

struct ifbpstpreq {
	uint8_t         ifbp_portno;            /* bp STP port number */
	uint32_t        ifbp_fwd_trans;         /* bp STP fwd transitions */
	uint32_t        ifbp_design_cost;       /* bp STP designated cost */
	uint32_t        ifbp_design_port;       /* bp STP designated port */
	uint64_t        ifbp_design_bridge;     /* bp STP designated bridge */
	uint64_t        ifbp_design_root;       /* bp STP designated root */
};

#pragma pack()

/*
 * Bridge STP ports list structure.
 */

#pragma pack(4)

#ifndef XNU_KERNEL_PRIVATE

struct ifbpstpconf {
	uint32_t        ifbpstp_len;    /* buffer size */
	union {
		caddr_t ifbpstpu_buf;
		struct ifbpstpreq *ifbpstpu_req;
	} ifbpstp_ifbpstpu;
#define ifbpstp_buf     ifbpstp_ifbpstpu.ifbpstpu_buf
#define ifbpstp_req     ifbpstp_ifbpstpu.ifbpstpu_req
};

#else /* XNU_KERNEL_PRIVATE */

struct ifbpstpconf32 {
	uint32_t        ifbpstp_len;    /* buffer size */
	union {
		user32_addr_t   ifbpstpu_buf;
		user32_addr_t   ifbpstpu_req;
#define ifbpstp_buf     ifbpstp_ifbpstpu.ifbpstpu_buf
#define ifbpstp_req     ifbpstp_ifbpstpu.ifbpstpu_req
	} ifbpstp_ifbpstpu;
};

struct ifbpstpconf64 {
	uint32_t        ifbpstp_len;    /* buffer size */
	union {
		user64_addr_t   ifbpstpu_buf;
		user64_addr_t   ifbpstpu_req;
	} ifbpstp_ifbpstpu;
};

#endif /* XNU_KERNEL_PRIVATE */

#pragma pack()

/*
 * Bridge member host filter.
 */

#define IFBRHF_ENABLED  0x01
#define IFBRHF_HWSRC    0x02    /* Valid with enabled flags */
#define IFBRHF_IPSRC    0x04    /* Valid with enabled flags */

#pragma pack(4)

struct ifbrhostfilter {
	uint32_t        ifbrhf_flags;           /* flags */
	char            ifbrhf_ifsname[IFNAMSIZ];       /* member if name */
	uint8_t         ifbrhf_hwsrca[ETHER_ADDR_LEN];
	uint32_t        ifbrhf_ipsrc;
};

#pragma pack()

/*
 * sysctl net.link.bridge.hostfilterstats
 */
struct bridge_hostfilter_stats {
	uint64_t        brhf_bad_ether_type;
	uint64_t        brhf_bad_ether_srchw_addr;

	uint64_t        brhf_ether_too_small;
	uint64_t        brhf_ether_pullup_failed;

	uint64_t        brhf_arp_ok;
	uint64_t        brhf_arp_too_small;
	uint64_t        brhf_arp_pullup_failed;
	uint64_t        brhf_arp_bad_hw_type;
	uint64_t        brhf_arp_bad_pro_type;
	uint64_t        brhf_arp_bad_hw_len;
	uint64_t        brhf_arp_bad_pro_len;
	uint64_t        brhf_arp_bad_op;
	uint64_t        brhf_arp_bad_sha;
	uint64_t        brhf_arp_bad_spa;

	uint64_t        brhf_ip_ok;
	uint64_t        brhf_ip_too_small;
	uint64_t        brhf_ip_pullup_failed;
	uint64_t        brhf_ip_bad_srcaddr;
	uint64_t        brhf_ip_bad_proto;

	uint64_t        brhf_dhcp_too_small;
	uint64_t        brhf_dhcp_bad_op;
	uint64_t        brhf_dhcp_bad_htype;
	uint64_t        brhf_dhcp_bad_hlen;
	uint64_t        brhf_dhcp_bad_chaddr;
	uint64_t        brhf_dhcp_bad_ciaddr;
};

#ifdef XNU_KERNEL_PRIVATE

extern u_int8_t bstp_etheraddr[ETHER_ADDR_LEN];

int     bridgeattach(int);

#endif /* XNU_KERNEL_PRIVATE */
#endif /* PRIVATE */
#endif /* !_NET_IF_BRIDGEVAR_H_ */
