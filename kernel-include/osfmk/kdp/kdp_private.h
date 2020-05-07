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
 * Private functions for kdp.c
 */

static boolean_t
kdp_unknown(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_connect(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_disconnect(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_reattach(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_hostinfo(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_suspend(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_readregs(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_writeregs(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_version(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_kernelversion(
	kdp_pkt_t             *,
	int                   *,
	unsigned short        *
	);

static boolean_t
kdp_regions(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_maxbytes(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_readmem(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_readmem64(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_readphysmem64(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_writemem(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_writemem64(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_writephysmem64(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_resumecpus(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_breakpoint_set(
	kdp_pkt_t *,
	int *,
	unsigned short *t
	);

static boolean_t
kdp_breakpoint64_set(
	kdp_pkt_t *,
	int  *,
	unsigned short *t
	);


static boolean_t
kdp_breakpoint_remove(
	kdp_pkt_t *,
	int *,
	unsigned short *
	);

static boolean_t
kdp_breakpoint64_remove(
	kdp_pkt_t *,
	int   *,
	unsigned short *
	);


static boolean_t
kdp_reboot(
	kdp_pkt_t *,
	int   *,
	unsigned short *
	);

static boolean_t
kdp_readioport(kdp_pkt_t *, int *, unsigned short *);

static boolean_t
kdp_writeioport(kdp_pkt_t *, int *, unsigned short *);

static boolean_t
kdp_readmsr64(kdp_pkt_t *, int *, unsigned short *);

static boolean_t
kdp_writemsr64(kdp_pkt_t *, int *, unsigned short *);

static boolean_t
kdp_dumpinfo(kdp_pkt_t *, int *, unsigned short *);
