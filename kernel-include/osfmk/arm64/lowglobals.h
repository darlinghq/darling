/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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

#ifndef __arm64__
#error  Wrong architecture - this file is meant for arm64
#endif

#define LOWGLO_LAYOUT_MAGIC             0xC0DEC0DE

/*
 * This structure is bound to lowmem_vectors.c. Make sure changes here are
 * reflected there as well.
 */

#pragma pack(8)         /* Make sure the structure stays as we defined it */
typedef struct lowglo {
	unsigned char   lgVerCode[8];           /* 0xffffff8000002000 System verification code */
	uint64_t                lgZero;                         /* 0xffffff8000002008 Constant 0 */
	uint64_t                lgStext;                        /* 0xffffff8000002010 Start of kernel text */
	uint64_t                lgVersion;                      /* 0xffffff8000002018 Pointer to kernel version string */
	uint64_t                lgOSVersion;            /* 0xffffff8000002020 Pointer to OS version string */
	uint64_t                lgKmodptr;                      /* 0xffffff8000002028 Pointer to kmod, debugging aid */
	uint64_t                lgTransOff;                     /* 0xffffff8000002030 Pointer to kdp_trans_off, debugging aid */
	uint64_t                lgRebootFlag;           /* 0xffffff8000002038 Pointer to debugger reboot trigger */
	uint64_t                lgManualPktAddr;        /* 0xffffff8000002040 Pointer to manual packet structure */
	uint64_t                lgAltDebugger;          /* 0xffffff8000002048 Pointer to reserved space for alternate kernel debugger */
	uint64_t                lgPmapMemQ;                     /* 0xffffff8000002050 Pointer to PMAP memory queue */
	uint64_t                lgPmapMemPageOffset;/* 0xffffff8000002058 Offset of physical page member in vm_page_t or vm_page_with_ppnum_t */
	uint64_t                lgPmapMemChainOffset;/*0xffffff8000002060 Offset of listq in vm_page_t or vm_page_with_ppnum_t  */
	uint64_t                lgStaticAddr;           /* 0xffffff8000002068 Static allocation address */
	uint64_t                lgStaticSize;           /* 0xffffff8000002070 Static allocation size */
	uint64_t                lgLayoutMajorVersion;   /* 0xffffff8000002078 Lowglo major layout version */
	uint64_t                lgLayoutMagic;          /* 0xffffff8000002080 Magic value evaluated to determine if lgLayoutVersion is valid */
	uint64_t                lgPmapMemStartAddr;     /* 0xffffff8000002088 Pointer to start of vm_page_t array */
	uint64_t                lgPmapMemEndAddr;       /* 0xffffff8000002090 Pointer to end of vm_page_t array */
	uint64_t                lgPmapMemPagesize;      /* 0xffffff8000002098 size of vm_page_t */
	uint64_t                lgPmapMemFromArrayMask; /* 0xffffff80000020A0 Mask to indicate page is from vm_page_t array */
	uint64_t                lgPmapMemFirstppnum;    /* 0xffffff80000020A8 physical page number of the first vm_page_t in the array */
	uint64_t                lgPmapMemPackedShift;   /* 0xffffff80000020B0 alignment of packed pointer */
	uint64_t                lgPmapMemPackedBaseAddr;/* 0xffffff80000020B8 base address of that packed pointers are relative to */
	uint64_t                lgLayoutMinorVersion;   /* 0xffffff80000020C0 Lowglo minor layout version */
	uint64_t                lgPageShift;            /* 0xffffff80000020C8 number of shifts from page number to size */
} lowglo;
#pragma pack()

extern lowglo lowGlo;

void patch_low_glo(void);
void patch_low_glo_static_region(uint64_t address, uint64_t size);
void patch_low_glo_vm_page_info(void *, void *, uint32_t);

#endif /* _LOW_MEMORY_GLOBALS_H_ */
