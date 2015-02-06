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
/*	$NetBSD: if_media.h,v 1.3 1997/03/26 01:19:27 thorpej Exp $	*/
/* $FreeBSD: src/sys/net/if_media.h,v 1.9.2.1 2001/07/04 00:12:38 brooks Exp $ */

/*
 * Copyright (c) 1997
 *	Jonathan Stone and Jason R. Thorpe.  All rights reserved.
 *
 * This software is derived from information provided by Matt Thomas.
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
 *	This product includes software developed by Jonathan Stone
 *	and Jason R. Thorpe for the NetBSD Project.
 * 4. The names of the authors may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _NET_IF_MEDIA_H_
#define _NET_IF_MEDIA_H_
#include <sys/appleapiopts.h>

/*
 * Prototypes and definitions for BSD/OS-compatible network interface
 * media selection.
 *
 * Where it is safe to do so, this code strays slightly from the BSD/OS
 * design.  Software which uses the API (device drivers, basically)
 * shouldn't notice any difference.
 *
 * Many thanks to Matt Thomas for providing the information necessary
 * to implement this interface.
 */


/*
 * if_media Options word:
 *	Bits	Use
 *	----	-------
 *	0-4	    Media subtype
 *	5-7     Media type
 *	8-15	Type specific options
 *	16-19	RFU
 *	20-27	Shared (global) options
 *	28-31	Instance
 */

/*
 * Ethernet
 */
#define IFM_ETHER	0x00000020
#define	IFM_10_T	3		/* 10BaseT - RJ45 */
#define	IFM_10_2	4		/* 10Base2 - Thinnet */
#define	IFM_10_5	5		/* 10Base5 - AUI */
#define	IFM_100_TX	6		/* 100BaseTX - RJ45 */
#define	IFM_100_FX	7		/* 100BaseFX - Fiber */
#define	IFM_100_T4	8		/* 100BaseT4 - 4 pair cat 3 */
#define	IFM_100_VG	9		/* 100VG-AnyLAN */
#define	IFM_100_T2	10		/* 100BaseT2 */
#define	IFM_1000_SX	11		/* 1000BaseSX - multi-mode fiber */
#define IFM_10_STP	12		/* 10BaseT over shielded TP */
#define IFM_10_FL	13		/* 10baseFL - Fiber */
#define	IFM_1000_LX	14		/* 1000baseLX - single-mode fiber */
#define	IFM_1000_CX	15		/* 1000baseCX - 150ohm STP */
#define	IFM_1000_T	16		/* 1000baseT - 4 pair cat 5 */
#define	IFM_HPNA_1	17		/* HomePNA 1.0 (1Mb/s) */
#define	IFM_10G_SR	18		/* 10GbaseSR - multi-mode fiber */
#define	IFM_10G_LR	19		/* 10GbaseLR - single-mode fiber */
#define	IFM_10G_CX4	20		/* 10GbaseCX4 - copper */
#define	IFM_10G_T	21		/* 10GbaseT - 4 pair cat 6 */

/*
 * Token ring
 */
#define	IFM_TOKEN	0x00000040
#define	IFM_TOK_STP4	3		/* Shielded twisted pair 4m - DB9 */
#define	IFM_TOK_STP16	4		/* Shielded twisted pair 16m - DB9 */
#define	IFM_TOK_UTP4	5		/* Unshielded twisted pair 4m - RJ45 */
#define	IFM_TOK_UTP16	6		/* Unshielded twisted pair 16m - RJ45 */
#define IFM_TOK_STP100  7		/* Shielded twisted pair 100m - DB9 */
#define IFM_TOK_UTP100  8		/* Unshielded twisted pair 100m - RJ45 */
#define	IFM_TOK_ETR	0x00000200	/* Early token release */
#define	IFM_TOK_SRCRT	0x00000400	/* Enable source routing features */
#define	IFM_TOK_ALLR	0x00000800	/* All routes / Single route bcast */
#define IFM_TOK_DTR	0x00002000	/* Dedicated token ring */
#define IFM_TOK_CLASSIC	0x00004000	/* Classic token ring */
#define IFM_TOK_AUTO	0x00008000	/* Automatic Dedicate/Classic token ring */

/*
 * FDDI
 */
#define	IFM_FDDI	0x00000060
#define	IFM_FDDI_SMF	3		/* Single-mode fiber */
#define	IFM_FDDI_MMF	4		/* Multi-mode fiber */
#define IFM_FDDI_UTP	5		/* CDDI / UTP */
#define IFM_FDDI_DA	0x00000100	/* Dual attach / single attach */

/*
 * IEEE 802.11 Wireless
 */
#define	IFM_IEEE80211	0x00000080
#define	IFM_IEEE80211_FH1	3	/* Frequency Hopping 1Mbps */
#define	IFM_IEEE80211_FH2	4	/* Frequency Hopping 2Mbps */
#define	IFM_IEEE80211_DS2	5	/* Direct Sequence 2Mbps */
#define	IFM_IEEE80211_DS5	6	/* Direct Sequence 5Mbps*/
#define	IFM_IEEE80211_DS11	7	/* Direct Sequence 11Mbps*/
#define	IFM_IEEE80211_DS1	8	/* Direct Sequence 1Mbps */
#define IFM_IEEE80211_DS22	9	/* Direct Sequence 22Mbps */
#define	IFM_IEEE80211_ADHOC	0x00000100	/* Operate in Adhoc mode */

/*
 * Shared media sub-types
 */
#define	IFM_AUTO	0		/* Autoselect best media */
#define	IFM_MANUAL	1		/* Jumper/dipswitch selects media */
#define	IFM_NONE	2		/* Deselect all media */

/*
 * Shared options
 */
#define IFM_FDX		0x00100000	/* Force full duplex */
#define	IFM_HDX		0x00200000	/* Force half duplex */
#define	IFM_FLOW	0x00400000	/* enable hardware flow control */
#define IFM_EEE		0x00800000	/* Support energy efficient ethernet */
#define IFM_FLAG0	0x01000000	/* Driver defined flag */
#define IFM_FLAG1	0x02000000	/* Driver defined flag */
#define IFM_FLAG2	0x04000000	/* Driver defined flag */
#define	IFM_LOOP	0x08000000	/* Put hardware in loopback */

/*
 * Masks
 */
#define	IFM_NMASK	0x000000e0	/* Network type */
#define	IFM_TMASK	0x0000001f	/* Media sub-type */
#define	IFM_IMASK	0xf0000000	/* Instance */
#define	IFM_ISHIFT	28		/* Instance shift */
#define	IFM_OMASK	0x0000ff00	/* Type specific options */
#define	IFM_GMASK	0x0ff00000	/* Global options */

/*
 * Status bits
 */
#define	IFM_AVALID	0x00000001	/* Active bit valid */
#define	IFM_ACTIVE	0x00000002	/* Interface attached to working net */

/*
 * Macros to extract various bits of information from the media word.
 */
#define	IFM_TYPE(x)         ((x) & IFM_NMASK)
#define	IFM_SUBTYPE(x)      ((x) & IFM_TMASK)
#define IFM_TYPE_OPTIONS(x) ((x) & IFM_OMASK)
#define	IFM_INST(x)         (((x) & IFM_IMASK) >> IFM_ISHIFT)
#define IFM_OPTIONS(x)	((x) & (IFM_OMASK|IFM_GMASK))

#define IFM_INST_MAX	IFM_INST(IFM_IMASK)

/*
 * Macro to create a media word.
 */
#define IFM_MAKEWORD(type, subtype, options, instance)			\
	((type) | (subtype) | (options) | ((instance) << IFM_ISHIFT))

/*
 * NetBSD extension not defined in the BSDI API.  This is used in various
 * places to get the canonical description for a given type/subtype.
 *
 * NOTE: all but the top-level type descriptions must contain NO whitespace!
 * Otherwise, parsing these in ifconfig(8) would be a nightmare.
 */
struct ifmedia_description {
	int	ifmt_word;		/* word value; may be masked */
	const char *ifmt_string;	/* description */
};

#define IFM_TYPE_DESCRIPTIONS {                     \
    { IFM_ETHER,     "Ethernet"   },                \
    { IFM_TOKEN,     "Token ring" },                \
    { IFM_FDDI,      "FDDI"       },                \
    { IFM_IEEE80211, "IEEE802.11" },                \
    { 0, NULL },                                    \
}

#define IFM_SUBTYPE_ETHERNET_DESCRIPTIONS {         \
    { IFM_10_T,     "10baseT/UTP" },                \
    { IFM_10_2,     "10base2/BNC" },                \
    { IFM_10_5,     "10base5/AUI" },                \
    { IFM_100_TX,   "100baseTX"   },                \
    { IFM_100_FX,   "100baseFX"   },                \
    { IFM_100_T4,   "100baseT4"   },                \
    { IFM_100_VG,   "100baseVG"   },                \
    { IFM_100_T2,   "100baseT2"   },                \
    { IFM_1000_SX,  "1000baseSX"  },                \
    { IFM_10_STP,   "10baseSTP"   },                \
    { IFM_10_FL,    "10baseFL"    },                \
    { IFM_1000_LX,  "1000baseLX"  },                \
    { IFM_1000_CX,  "1000baseCX"  },                \
    { IFM_1000_T,   "1000baseT"   },                \
    { IFM_HPNA_1,   "HomePNA1"    },                \
    { IFM_10G_SR,   "10GbaseSR"   },                \
    { IFM_10G_LR,   "10GbaseLR"   },                \
    { IFM_10G_CX4,  "10GbaseCX4"  },                \
    { IFM_10G_T,    "10GbaseT"    },                \
    { 0, NULL },                                    \
}

#define IFM_SUBTYPE_ETHERNET_ALIASES {              \
    { IFM_10_T,     "UTP"    },                     \
    { IFM_10_T,     "10UTP"  },                     \
    { IFM_10_2,     "BNC"    },                     \
    { IFM_10_2,     "10BNC"  },                     \
    { IFM_10_5,     "AUI"    },                     \
    { IFM_10_5,     "10AUI"  },                     \
    { IFM_100_TX,   "100TX"  },                     \
    { IFM_100_FX,   "100FX"  },                     \
    { IFM_100_T4,   "100T4"  },                     \
    { IFM_100_VG,   "100VG"  },                     \
    { IFM_100_T2,   "100T2"  },                     \
    { IFM_1000_SX,  "1000SX" },                     \
    { IFM_10_STP,   "STP"    },                     \
    { IFM_10_STP,   "10STP"  },                     \
    { IFM_10_FL,    "FL"     },                     \
    { IFM_10_FL,    "10FL"   },                     \
    { IFM_1000_LX,  "1000LX" },                     \
    { IFM_1000_CX,  "1000CX" },                     \
    { IFM_1000_T,   "1000T"  },                     \
    { IFM_HPNA_1,   "HPNA1"  },                     \
    { IFM_10G_SR,   "10GSR"  },                     \
    { IFM_10G_LR,   "10GLR"  },                     \
    { IFM_10G_CX4,  "10GCX4" },                     \
    { IFM_10G_T,    "10GT"   },                     \
    { 0, NULL },                                    \
}

#define IFM_SUBTYPE_ETHERNET_OPTION_DESCRIPTIONS {  \
    { 0, NULL },                                    \
}

#define IFM_SUBTYPE_TOKENRING_DESCRIPTIONS {        \
    { IFM_TOK_STP4,  "DB9/4Mbit" },                 \
    { IFM_TOK_STP16, "DB9/16Mbit" },                \
    { IFM_TOK_UTP4,  "UTP/4Mbit" },                 \
    { IFM_TOK_UTP16, "UTP/16Mbit" },                \
    { 0, NULL },                                    \
}

#define IFM_SUBTYPE_TOKENRING_ALIASES {             \
    { IFM_TOK_STP4,  "4STP" },                      \
    { IFM_TOK_STP16, "16STP" },                     \
    { IFM_TOK_UTP4,  "4UTP" },                      \
    { IFM_TOK_UTP16, "16UTP" },                     \
    { 0, NULL },                                    \
}

#define IFM_SUBTYPE_TOKENRING_OPTION_DESCRIPTIONS { \
    { IFM_TOK_ETR,   "EarlyTokenRelease" },         \
    { IFM_TOK_SRCRT, "SourceRouting" },             \
    { IFM_TOK_ALLR,  "AllRoutes" },                 \
    { 0, NULL },                                    \
}

#define IFM_SUBTYPE_FDDI_DESCRIPTIONS {             \
    { IFM_FDDI_SMF, "Single-mode" },                \
    { IFM_FDDI_MMF, "Multi-mode" },                 \
    { IFM_FDDI_UTP, "UTP" },                        \
    { 0, NULL },                                    \
}

#define IFM_SUBTYPE_FDDI_ALIASES {                  \
    { IFM_FDDI_SMF, "SMF" },                        \
    { IFM_FDDI_MMF, "MMF" },                        \
    { IFM_FDDI_UTP, "CDDI" },                       \
    { 0, NULL },                                    \
}

#define IFM_SUBTYPE_FDDI_OPTION_DESCRIPTIONS {      \
    { IFM_FDDI_DA,  "Dual-attach" },                \
    { 0, NULL },                                    \
}

#define IFM_SUBTYPE_IEEE80211_DESCRIPTIONS {        \
    { IFM_IEEE80211_FH1,  "FH1"  },                 \
    { IFM_IEEE80211_FH2,  "FH2"  },                 \
    { IFM_IEEE80211_DS1,  "DS1"  },                 \
    { IFM_IEEE80211_DS2,  "DS2"  },                 \
    { IFM_IEEE80211_DS5,  "DS5"  },                 \
    { IFM_IEEE80211_DS11, "DS11" },                 \
    { IFM_IEEE80211_DS22, "DS22" },                 \
    { 0, NULL },                                    \
}

#define IFM_SUBTYPE_IEEE80211_OPTION_DESCRIPTIONS { \
    { IFM_IEEE80211_ADHOC,  "adhoc" },              \
    { 0, NULL },                                    \
}

#define IFM_SUBTYPE_SHARED_DESCRIPTIONS {           \
    { IFM_AUTO,     "autoselect" },                 \
    { IFM_MANUAL,   "manual" },                     \
    { IFM_NONE,     "none" },                       \
    { 0, NULL },                                    \
}

#define IFM_SUBTYPE_SHARED_ALIASES {                \
    { IFM_AUTO,     "auto" },                       \
    { 0, NULL },                                    \
}

#define IFM_SHARED_OPTION_DESCRIPTIONS {            \
    { IFM_FDX,      "full-duplex" },                \
    { IFM_HDX,      "half-duplex" },                \
    { IFM_FLOW,     "flow-control" },               \
    { IFM_EEE,	    "energy-efficient-ethernet" },  \
    { IFM_FLAG0,    "flag0" },                      \
    { IFM_FLAG1,    "flag1" },                      \
    { IFM_FLAG2,    "flag2" },                      \
    { IFM_LOOP,     "hw-loopback" },                \
    { 0, NULL },                                    \
}

#endif	/* _NET_IF_MEDIA_H_ */
