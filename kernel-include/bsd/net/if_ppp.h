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
 * if_ppp.h - Point-to-Point Protocol definitions.
 *
 * Copyright (c) 1989 Carnegie Mellon University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Carnegie Mellon University.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $FreeBSD: src/sys/net/if_ppp.h,v 1.14 1999/08/28 00:48:20 peter Exp $
 */

#ifndef _IF_PPP_H_
#define _IF_PPP_H_
#include <sys/appleapiopts.h>

/* XXX this used to be self-contained. */
#include <net/ppp_defs.h>
#include <net/if.h>

/*
 * Packet sizes
 */
#define PPP_MTU         1500    /* Default MTU (size of Info field) */
#define PPP_MAXMRU      65000   /* Largest MRU we allow */
#define PPP_MAXMTU      16384   /* Largest MTU we allow */

/*
 * Bit definitions for flags.
 */
#define SC_COMP_PROT    0x00000001      /* protocol compression (output) */
#define SC_COMP_AC      0x00000002      /* header compression (output) */
#define SC_COMP_TCP     0x00000004      /* TCP (VJ) compression (output) */
#define SC_NO_TCP_CCID  0x00000008      /* disable VJ connection-id comp. */
#define SC_REJ_COMP_AC  0x00000010      /* reject adrs/ctrl comp. on input */
#define SC_REJ_COMP_TCP 0x00000020      /* reject TCP (VJ) comp. on input */
#define SC_CCP_OPEN     0x00000040      /* Look at CCP packets */
#define SC_CCP_UP       0x00000080      /* May send/recv compressed packets */
#define SC_DEBUG        0x00010000      /* enable debug messages */
#define SC_LOG_INPKT    0x00020000      /* log contents of good pkts recvd */
#define SC_LOG_OUTPKT   0x00040000      /* log contents of pkts sent */
#define SC_LOG_RAWIN    0x00080000      /* log all chars received */
#define SC_LOG_FLUSH    0x00100000      /* log all chars flushed */
#define SC_RCV_B7_0     0x01000000      /* have rcvd char with bit 7 = 0 */
#define SC_RCV_B7_1     0x02000000      /* have rcvd char with bit 7 = 1 */
#define SC_RCV_EVNP     0x04000000      /* have rcvd char with even parity */
#define SC_RCV_ODDP     0x08000000      /* have rcvd char with odd parity */
#define SC_MASK         0x0fff00ff      /* bits that user can change */

/*
 * State bits in sc_flags, not changeable by user.
 */
#define SC_TIMEOUT      0x00000400      /* timeout is currently pending */
#define SC_VJ_RESET     0x00000800      /* need to reset VJ decomp */
#define SC_COMP_RUN     0x00001000      /* compressor has been initiated */
#define SC_DECOMP_RUN   0x00002000      /* decompressor has been initiated */
#define SC_DC_ERROR     0x00004000      /* non-fatal decomp error detected */
#define SC_DC_FERROR    0x00008000      /* fatal decomp error detected */
#define SC_TBUSY        0x10000000      /* xmitter doesn't need a packet yet */
#define SC_PKTLOST      0x20000000      /* have lost or dropped a packet */
#define SC_FLUSH        0x40000000      /* flush input until next PPP_FLAG */
#define SC_ESCAPED      0x80000000      /* saw a PPP_ESCAPE */

/*
 * Ioctl definitions.
 */

struct npioctl {
	int         protocol;   /* PPP procotol, e.g. PPP_IP */
	enum NPmode mode;
};

/* Structure describing a CCP configuration option, for PPPIOCSCOMPRESS */
struct ppp_option_data {
	u_char  *ptr;
	u_int   length;
	int     transmit;
};

struct ifpppstatsreq {
	char ifr_name[IFNAMSIZ];
	struct ppp_stats stats;
};

struct ifpppcstatsreq {
	char ifr_name[IFNAMSIZ];
	struct ppp_comp_stats stats;
};

/*
 * Ioctl definitions.
 */

#define PPPIOCGFLAGS    _IOR('t', 90, int)      /* get configuration flags */
#define PPPIOCSFLAGS    _IOW('t', 89, int)      /* set configuration flags */
#define PPPIOCGASYNCMAP _IOR('t', 88, int)      /* get async map */
#define PPPIOCSASYNCMAP _IOW('t', 87, int)      /* set async map */
#define PPPIOCGUNIT     _IOR('t', 86, int)      /* get ppp unit number */
#define PPPIOCGRASYNCMAP _IOR('t', 85, int)     /* get receive async map */
#define PPPIOCSRASYNCMAP _IOW('t', 84, int)     /* set receive async map */
#define PPPIOCGMRU      _IOR('t', 83, int)      /* get max receive unit */
#define PPPIOCSMRU      _IOW('t', 82, int)      /* set max receive unit */
#define PPPIOCSMAXCID   _IOW('t', 81, int)      /* set VJ max slot ID */
#define PPPIOCGXASYNCMAP _IOR('t', 80, ext_accm) /* get extended ACCM */
#define PPPIOCSXASYNCMAP _IOW('t', 79, ext_accm) /* set extended ACCM */
#define PPPIOCXFERUNIT  _IO('t', 78)            /* transfer PPP unit */
#define PPPIOCSCOMPRESS _IOW('t', 77, struct ppp_option_data)
#define PPPIOCGNPMODE   _IOWR('t', 76, struct npioctl) /* get NP mode */
#define PPPIOCSNPMODE   _IOW('t', 75, struct npioctl)  /* set NP mode */
#define PPPIOCGIDLE     _IOR('t', 74, struct ppp_idle) /* get idle time */
#define PPPIOCSPASS     _IOW('t', 71, struct bpf_program) /* set pass filter */
#define PPPIOCSACTIVE   _IOW('t', 70, struct bpf_program) /* set active filt */

/* PPPIOC[GS]MTU are alternatives to SIOC[GS]IFMTU, used under Ultrix */
#define PPPIOCGMTU      _IOR('t', 73, int)      /* get interface MTU */
#define PPPIOCSMTU      _IOW('t', 72, int)      /* set interface MTU */

/*
 * These two are interface ioctls so that pppstats can do them on
 * a socket without having to open the serial device.
 */
#define SIOCGPPPSTATS   _IOWR('i', 123, struct ifpppstatsreq)
#define SIOCGPPPCSTATS  _IOWR('i', 122, struct ifpppcstatsreq)

#if !defined(ifr_mtu)
#define ifr_mtu ifr_ifru.ifru_metric
#endif

#endif /* _IF_PPP_H_ */
