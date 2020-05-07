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

#ifndef __arm__
#error  Wrong architecture - this file is meant for arm
#endif

#define LOWGLO_LAYOUT_MAGIC             0xC0DEC0DE

#pragma pack(4)         /* Make sure the structure stays as we defined it */
typedef struct lowglo {
	unsigned char           lgVerCode[8];           /* 0xffff1000 System verification code */
	uint32_t                lgZero[2];                      /* 0xffff1008 Double constant 0 */
	uint32_t                lgStext;                        /* 0xffff1010 Start of kernel text */
	uint32_t                lgRsv014[2];            /* 0xffff1014 Reserved */
	uint32_t                lgVersion;                      /* 0xffff101C Pointer to kernel version string */
	uint32_t                lgRsv020[216];          /* 0xffff1020 Reserved */
	uint32_t                lgKmodptr;                      /* 0xffff1380 Pointer to kmod, debugging aid */
	uint32_t                lgTransOff;                     /* 0xffff1384 Pointer to kdp_trans_off, debugging aid */
	uint32_t                lgRsv388[3];            /* 0xffff1388 Reserved */
	uint32_t                lgOSVersion;            /* 0xffff1394 Pointer to OS version string */
	uint32_t                lgRsv398;                       /* 0xffff1398 Reserved */
	uint32_t                lgRebootFlag;           /* 0xffff139C Pointer to debugger reboot trigger */
	uint32_t                lgManualPktAddr;        /* 0xffff13A0 Pointer to manual packet structure */
	uint32_t                lgRsv3A4;                       /* 0xffff13A4 Reserved */
	uint32_t                lgPmapMemQ;                     /* 0xffff13A8 Pointer to PMAP memory queue */
	uint32_t                lgPmapMemPageOffset;/* 0xffff13AC Offset of physical page member in vm_page_with_ppnum_t */
	uint32_t                lgPmapMemChainOffset;/*0xffff13B0 Offset of listq in vm_page_t or vm_page_with_ppnum_t */
	uint32_t                lgStaticAddr;           /* 0xffff13B4 Static allocation address */
	uint32_t                lgStaticSize;           /* 0xffff13B8 Static allocation size */
	uint32_t                lgLayoutMajorVersion;   /* 0xffff13BC Lowglo layout major version */
	uint32_t                lgLayoutMagic;          /* 0xffff13C0 Magic value evaluated to determine if lgLayoutVersion is valid */
	uint32_t                lgPmapMemStartAddr;     /* 0xffff13C4 Pointer to start of vm_page_t array */
	uint32_t                lgPmapMemEndAddr;       /* 0xffff13C8 Pointer to end of vm_page_t array */
	uint32_t                lgPmapMemPagesize;      /* 0xffff13CC size of vm_page_t */
	uint32_t                lgPmapMemFirstppnum;    /* 0xffff13D0 physical page number of the first vm_page_t in the array */
	uint32_t                lgLayoutMinorVersion;   /* 0xffff13D4 Lowglo layout minor version */
	uint32_t                lgPageShift;            /* 0xffff13D8 Number of shifts from page number to size */
} lowglo;
#pragma pack()

extern lowglo lowGlo;

void patch_low_glo(void);
void patch_low_glo_static_region(uint32_t address, uint32_t size);
void patch_low_glo_vm_page_info(void *, void *, uint32_t);

#endif /* _LOW_MEMORY_GLOBALS_H_ */
