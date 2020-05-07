/*
 * Copyright (c) 2000-2017 Apple Inc. All rights reserved.
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
 * Copyright (c) 1989, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)if_types.h	8.2 (Berkeley) 4/20/94
 * $FreeBSD: src/sys/net/if_types.h,v 1.8.2.3 2001/07/03 11:01:41 ume Exp $
 */

#ifndef _NET_IF_TYPES_H_
#define _NET_IF_TYPES_H_
#include <sys/appleapiopts.h>

/*
 * Interface types for benefit of parsing media address headers.
 * This list is derived from the SNMP list of ifTypes, currently
 * documented in RFC1573.
 * The current list of assignments is maintained at:
 *      http://www.iana.org/assignments/smi-numbers
 */

#define IFT_OTHER       0x1             /* none of the following */
#define IFT_1822        0x2             /* old-style arpanet imp */
#define IFT_HDH1822     0x3             /* HDH arpanet imp */
#define IFT_X25DDN      0x4             /* x25 to imp */
#define IFT_X25         0x5             /* PDN X25 interface (RFC877) */
#define IFT_ETHER       0x6             /* Ethernet CSMACD */
#define IFT_ISO88023    0x7             /* CMSA CD */
#define IFT_ISO88024    0x8             /* Token Bus */
#define IFT_ISO88025    0x9             /* Token Ring */
#define IFT_ISO88026    0xa             /* MAN */
#define IFT_STARLAN     0xb
#define IFT_P10         0xc             /* Proteon 10MBit ring */
#define IFT_P80         0xd             /* Proteon 80MBit ring */
#define IFT_HY          0xe             /* Hyperchannel */
#define IFT_FDDI        0xf
#define IFT_LAPB        0x10
#define IFT_SDLC        0x11
#define IFT_T1          0x12
#define IFT_CEPT        0x13            /* E1 - european T1 */
#define IFT_ISDNBASIC   0x14
#define IFT_ISDNPRIMARY 0x15
#define IFT_PTPSERIAL   0x16            /* Proprietary PTP serial */
#define IFT_PPP         0x17            /* RFC 1331 */
#define IFT_LOOP        0x18            /* loopback */
#define IFT_EON         0x19            /* ISO over IP */
#define IFT_XETHER      0x1a            /* obsolete 3MB experimental ethernet */
#define IFT_NSIP        0x1b            /* XNS over IP */
#define IFT_SLIP        0x1c            /* IP over generic TTY */
#define IFT_ULTRA       0x1d            /* Ultra Technologies */
#define IFT_DS3         0x1e            /* Generic T3 */
#define IFT_SIP         0x1f            /* SMDS */
#define IFT_FRELAY      0x20            /* Frame Relay DTE only */
#define IFT_RS232       0x21
#define IFT_PARA        0x22            /* parallel-port */
#define IFT_ARCNET      0x23
#define IFT_ARCNETPLUS  0x24
#define IFT_ATM         0x25            /* ATM cells */
#define IFT_MIOX25      0x26
#define IFT_SONET       0x27            /* SONET or SDH */
#define IFT_X25PLE      0x28
#define IFT_ISO88022LLC 0x29
#define IFT_LOCALTALK   0x2a
#define IFT_SMDSDXI     0x2b
#define IFT_FRELAYDCE   0x2c            /* Frame Relay DCE */
#define IFT_V35         0x2d
#define IFT_HSSI        0x2e
#define IFT_HIPPI       0x2f
#define IFT_MODEM       0x30            /* Generic Modem */
#define IFT_AAL5        0x31            /* AAL5 over ATM */
#define IFT_SONETPATH   0x32
#define IFT_SONETVT     0x33
#define IFT_SMDSICIP    0x34            /* SMDS InterCarrier Interface */
#define IFT_PROPVIRTUAL 0x35            /* Proprietary Virtual/internal */
#define IFT_PROPMUX     0x36            /* Proprietary Multiplexing */
/*
 * IFT_GIF, IFT_FAITH and IFT_6LOWPAN are not based on IANA assignments.
 * Note: IFT_STF has a defined ifType: 0xd7 (215), but we use 0x39.
 */
#define IFT_GIF         0x37            /*0xf0*/
#define IFT_FAITH       0x38            /*0xf2*/
#define IFT_STF         0x39            /*0xf3*/
#define IFT_6LOWPAN     0x40            /* IETF RFC 6282 */

#define IFT_L2VLAN      0x87            /* Layer 2 Virtual LAN using 802.1Q */
#define IFT_IEEE8023ADLAG 0x88          /* IEEE802.3ad Link Aggregate */
#define IFT_IEEE1394    0x90            /* IEEE1394 High Performance SerialBus*/
#define IFT_BRIDGE      0xd1            /* Transparent bridge interface */

#define IFT_ENC         0xf4            /* Encapsulation */
#define IFT_PFLOG       0xf5            /* Packet filter logging */
#define IFT_PFSYNC      0xf6            /* Packet filter state syncing */
#define IFT_CARP        0xf8            /* Common Address Redundancy Protocol */
#define IFT_PKTAP       0xfe            /* Packet tap pseudo interface */
#define IFT_CELLULAR    0xff            /* Packet Data over Cellular */
#define IFT_PDP         IFT_CELLULAR    /* deprecated; use IFT_CELLULAR */

#endif
