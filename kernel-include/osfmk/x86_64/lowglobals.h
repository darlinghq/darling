/*
 * Copyright (c) 2000-2006 Apple Computer, Inc. All rights reserved.
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
 *		Header files for the Low Memory Globals (lg)
 */
#ifndef _LOW_MEMORY_GLOBALS_H_
#define _LOW_MEMORY_GLOBALS_H_

#include <mach/mach_types.h>
#include <mach/vm_types.h>
#include <mach/machine/vm_types.h>
#include <mach/vm_prot.h>

#ifndef __x86_64__
#error  Wrong architecture - this file is meant for x86_64
#endif

/*
 * Don't change these structures unless you change the corresponding assembly code
 * which is in lowmem_vectors.s
 */

#pragma pack(8)         /* Make sure the structure stays as we defined it */
typedef struct lowglo {
	unsigned char   lgVerCode[8];           /* 0xffffff8000002000 System verification code */
	uint64_t        lgZero;                 /* 0xffffff8000002008 Double constant 0 */
	uint64_t        lgStext;                /* 0xffffff8000002010 Start of kernel text */
	uint64_t        lgRsv018;               /* 0xffffff8000002018 Reserved */
	uint64_t        lgRsv020;               /* 0xffffff8000002020 Reserved */
	uint64_t        lgRsv028;               /* 0xffffff8000002028 Reserved */
	uint64_t        lgVersion;              /* 0xffffff8000002030 Pointer to kernel version string */
	uint64_t        lgRsv038[280];          /* 0xffffff8000002038 Reserved */
	uint64_t        lgKmodptr;              /* 0xffffff80000028f8 Pointer to kmod, debugging aid */
	uint64_t        lgTransOff;             /* 0xffffff8000002900 Pointer to kdp_trans_off, debugging aid */
	uint64_t        lgReadIO;               /* 0xffffff8000002908 Pointer to kdp_read_io, debugging aid */
	uint64_t        lgDevSlot1;             /* 0xffffff8000002910 For developer use */
	uint64_t        lgDevSlot2;             /* 0xffffff8000002918 For developer use */
	uint64_t        lgOSVersion;            /* 0xffffff8000002920 Pointer to OS version string */
	uint64_t        lgRebootFlag;           /* 0xffffff8000002928 Pointer to debugger reboot trigger */
	uint64_t        lgManualPktAddr;        /* 0xffffff8000002930 Pointer to manual packet structure */
	uint64_t        lgKdpJtagCoredumpAddr;  /* 0xffffff8000002938 Pointer to kdp_jtag_coredump_t structure */

	uint64_t        lgRsv940[216];          /* 0xffffff8000002940 Reserved - push to 1 page */
} lowglo;
#pragma pack()
extern lowglo lowGlo;
#endif /* _LOW_MEMORY_GLOBALS_H_ */
