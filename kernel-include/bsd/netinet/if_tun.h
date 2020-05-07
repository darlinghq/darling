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
/* Copyright (c) 1997 Apple Computer, Inc. All Rights Reserved */
/*
 * Copyright (c) 1988, Julian Onions <jpo@cs.nott.ac.uk>
 * Nottingham University 1987.
 *
 * This source may be freely distributed, however I would be interested
 * in any changes that are made.
 *
 * This driver takes packets off the IP i/f and hands them up to a
 * user process to have it's wicked way with. This driver has it's
 * roots in a similar driver written by Phil Cockcroft (formerly) at
 * UCL. This driver is based much more on read/write/select mode of
 * operation though.
 *
 */

#ifndef _NET_IF_TUN_H_
#define _NET_IF_TUN_H_
#include <sys/appleapiopts.h>

#ifdef KERNEL_PRIVATE
struct tun_softc {
	u_short tun_flags;              /* misc flags */
#define TUN_OPEN        0x0001
#define TUN_INITED      0x0002
#define TUN_RCOLL       0x0004
#define TUN_IASET       0x0008
#define TUN_DSTADDR     0x0010
#define TUN_RWAIT       0x0040
#define TUN_ASYNC       0x0080
#define TUN_NBIO        0x0100

#define TUN_READY       (TUN_OPEN | TUN_INITED | TUN_IASET)

	struct  ifnet tun_if;           /* the interface */
	int     tun_pgrp;               /* the process group - if any */
	struct  selinfo tun_rsel;       /* read select */
	struct  selinfo tun_wsel;       /* write select (not used) */
#if NBPFILTER > 0
	caddr_t         tun_bpf;
#endif
};

#endif /* KERNEL_PRIVATE */

/* ioctl's for get/set debug */
#define TUNSDEBUG       _IOW('t', 90, int)
#define TUNGDEBUG       _IOR('t', 89, int)

/* Maximum packet size */
#define TUNMTU          1500

#endif /* !_NET_IF_TUN_H_ */
