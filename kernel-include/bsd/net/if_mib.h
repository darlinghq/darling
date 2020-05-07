/*
 * Copyright (c) 2000-2019 Apple Inc. All rights reserved.
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
 * Copyright 1996 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that both the above copyright notice and this
 * permission notice appear in all copies, that both the above
 * copyright notice and this permission notice appear in all
 * supporting documentation, and that the name of M.I.T. not be used
 * in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  M.I.T. makes
 * no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THIS SOFTWARE IS PROVIDED BY M.I.T. ``AS IS''.  M.I.T. DISCLAIMS
 * ALL EXPRESS OR IMPLIED WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT
 * SHALL M.I.T. BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/sys/net/if_mib.h,v 1.6 1999/08/28 00:48:19 peter Exp $
 */

#ifndef _NET_IF_MIB_H
#define _NET_IF_MIB_H   1
#include <sys/appleapiopts.h>

struct ifmibdata {
	char                            ifmd_name[IFNAMSIZ]; /* name of interface */
	unsigned int            ifmd_pcount;    /* number of promiscuous listeners */
	unsigned int            ifmd_flags;     /* interface flags */
	unsigned int            ifmd_snd_len;   /* instantaneous length of send queue */
	unsigned int            ifmd_snd_maxlen; /* maximum length of send queue */
	unsigned int            ifmd_snd_drops; /* number of drops in send queue */
	unsigned int            ifmd_filler[4]; /* for future expansion */
	struct if_data64        ifmd_data; /* generic information and statistics */
};

#ifdef PRIVATE
struct ifmibdata_supplemental {
	struct if_traffic_class ifmd_traffic_class;
	struct if_data_extended ifmd_data_extended;
	struct if_packet_stats  ifmd_packet_stats;
	struct if_rxpoll_stats  ifmd_rxpoll_stats;
	struct if_netif_stats   ifmd_netif_stats;
};
#endif /* PRIVATE */

/*
 * sysctl MIB tags at the net.link.generic level
 */
#define IFMIB_SYSTEM    1       /* non-interface-specific */
#define IFMIB_IFDATA    2       /* per-interface data table */
#define IFMIB_IFALLDATA 3       /* all interfaces data at once */

/*
 * MIB tags for the various net.link.generic.ifdata tables
 */
#define IFDATA_GENERAL          1       /* generic stats for all kinds of ifaces */
#define IFDATA_LINKSPECIFIC     2       /* specific to the type of interface */
#define IFDATA_ADDRS            3       /* addresses assigned to interface */
#define IFDATA_MULTIADDRS       4       /* multicast addresses assigned to interface */
#ifdef PRIVATE
#define IFDATA_SUPPLEMENTAL     5       /* supplemental link specific stats */
#endif /* PRIVATE */

/*
 * MIB tags at the net.link.generic.system level
 */
#define IFMIB_IFCOUNT   1       /* number of interfaces configured */

/*
 * MIB tags as the net.link level
 * All of the other values are IFT_* names defined in if_types.h.
 */
#define NETLINK_GENERIC 0       /* functions not specific to a type of iface */

/*
 * The reason why the IFDATA_LINKSPECIFIC stuff is not under the
 * net.link.<iftype> branches is twofold:
 *   1) It's easier to code this way, and doesn't require duplication.
 *   2) The fourth level under net.link.<iftype> is <pf>; that is to say,
 *	the net.link.<iftype> tree instruments the adaptation layers between
 *	<iftype> and a particular protocol family (e.g., net.link.ether.inet
 *	instruments ARP).  This does not really leave room for anything else
 *	that needs to have a well-known number.
 */

/*
 * Link-specific MIB structures for various link types.
 */

/* For IFT_ETHER, IFT_ISO88023, and IFT_STARLAN, as used by RFC 1650 */
struct ifs_iso_8802_3 {
	u_int32_t       dot3StatsAlignmentErrors;
	u_int32_t       dot3StatsFCSErrors;
	u_int32_t       dot3StatsSingleCollisionFrames;
	u_int32_t       dot3StatsMultipleCollisionFrames;
	u_int32_t       dot3StatsSQETestErrors;
	u_int32_t       dot3StatsDeferredTransmissions;
	u_int32_t       dot3StatsLateCollisions;
	u_int32_t       dot3StatsExcessiveCollisions;
	u_int32_t       dot3StatsInternalMacTransmitErrors;
	u_int32_t       dot3StatsCarrierSenseErrors;
	u_int32_t       dot3StatsFrameTooLongs;
	u_int32_t       dot3StatsInternalMacReceiveErrors;
	u_int32_t       dot3StatsEtherChipSet;
	/* Matt Thomas wants this one, not included in RFC 1650: */
	u_int32_t       dot3StatsMissedFrames;

	u_int32_t       dot3StatsCollFrequencies[16]; /* NB: index origin */

	u_int32_t       dot3Compliance;
#define DOT3COMPLIANCE_STATS    1
#define DOT3COMPLIANCE_COLLS    2
};

/*
 * Chipset identifiers are normally part of the vendor's enterprise MIB.
 * However, we don't want to be trying to represent arbitrary-length
 * OBJECT IDENTIFIERs here (ick!), and the right value is not necessarily
 * obvious to the driver implementor.  So, we define our own identification
 * mechanism here, and let the agent writer deal with the translation.
 */
#define DOT3CHIPSET_VENDOR(x)   ((x) >> 16)
#define DOT3CHIPSET_PART(x)     ((x) & 0xffff)
#define DOT3CHIPSET(v, p)        (((v) << 16) + ((p) & 0xffff))

/* Driver writers!  Add your vendors here! */
enum dot3Vendors {
	dot3VendorAMD = 1,
	dot3VendorIntel = 2,
	dot3VendorNational = 4,
	dot3VendorFujitsu = 5,
	dot3VendorDigital = 6,
	dot3VendorWesternDigital = 7
};

/* Driver writers!  Add your chipsets here! */
enum {
	dot3ChipSetAMD7990 = 1,
	dot3ChipSetAMD79900 = 2,
	dot3ChipSetAMD79C940 = 3
};

enum {
	dot3ChipSetIntel82586 = 1,
	dot3ChipSetIntel82596 = 2,
	dot3ChipSetIntel82557 = 3
};

enum {
	dot3ChipSetNational8390 = 1,
	dot3ChipSetNationalSonic = 2
};

enum {
	dot3ChipSetFujitsu86950 = 1
};

enum {
	dot3ChipSetDigitalDC21040 = 1,
	dot3ChipSetDigitalDC21140 = 2,
	dot3ChipSetDigitalDC21041 = 3,
	dot3ChipSetDigitalDC21140A = 4,
	dot3ChipSetDigitalDC21142 = 5
};

enum {
	dot3ChipSetWesternDigital83C690 = 1,
	dot3ChipSetWesternDigital83C790 = 2
};
/* END of Ethernet-link MIB stuff */

/*
 * Put other types of interface MIBs here, or in interface-specific
 * header files if convenient ones already exist.
 */

/*
 * Structure for interface family ID table
 */

struct if_family_id {
	u_int32_t               iffmid_len;
	u_int32_t               iffmid_id;
	char                    iffmid_str[1];  /* variable length string */
};


#endif /* _NET_IF_MIB_H */
