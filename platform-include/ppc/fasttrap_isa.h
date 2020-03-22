/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef	_FASTTRAP_ISA_H
#define	_FASTTRAP_ISA_H

/* #pragma ident	"@(#)fasttrap_isa.h	1.4	05/06/08 SMI" */

#include <sys/types.h>
#if defined(__APPLE__)
#include <stdint.h>
#endif 

#ifdef	__cplusplus
extern "C" {
#endif

typedef	uint32_t		fasttrap_instr_t;

typedef struct fasttrap_machtp {
	fasttrap_instr_t	ftmt_instr;     /* Original instruction */
	int32_t				ftmt_trgt;		/* Offset or absolute address */
	uint8_t				ftmt_type;		/* Emulation function type */
#define ftmtNOP 0
#define ftmtCommon 1
#define ftmtB 2
#define ftmtBC 3
#define ftmtBLR 4
#define ftmtBCTR 5
	uint8_t				ftmt_bo;		/* Branch options */
	uint8_t				ftmt_bi;		/* Condition bit */
	uint8_t				ftmt_flgs;		/* Flags */
#define ftmtAbs 2
#define ftmtLink 1
} fasttrap_machtp_t;

#define	ftt_instr	ftt_mtp.ftmt_instr
#define	ftt_trgt	ftt_mtp.ftmt_trgt
#define	ftt_type	ftt_mtp.ftmt_type
#define	ftt_bo		ftt_mtp.ftmt_bo
#define	ftt_bi		ftt_mtp.ftmt_bi
#define	ftt_flgs	ftt_mtp.ftmt_flgs

#define	FASTTRAP_INSTR 0x0FFFDDDD
#define T_DTRACE_RET (0x2E * 4)

#define	FASTTRAP_RETURN_AFRAMES		7
#define	FASTTRAP_ENTRY_AFRAMES		7
#define	FASTTRAP_OFFSET_AFRAMES		6

#ifdef	__cplusplus
}
#endif

#endif	/* _FASTTRAP_ISA_H */
