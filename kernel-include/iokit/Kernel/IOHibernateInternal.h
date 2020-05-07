/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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

#include <stdint.h>

#ifdef __cplusplus

enum { kIOHibernateAESKeySize = 128 };  /* bits */

struct IOHibernateVars {
	hibernate_page_list_t *             page_list;
	hibernate_page_list_t *             page_list_wired;
	hibernate_page_list_t *             page_list_pal;
	class IOBufferMemoryDescriptor *    ioBuffer;
	class IOBufferMemoryDescriptor *    srcBuffer;
	class IOBufferMemoryDescriptor *    handoffBuffer;
	class IOMemoryDescriptor *          previewBuffer;
	OSData *                            previewData;
	OSObject *                          saveBootDevice;

	struct IOPolledFileIOVars *         fileVars;
	uint64_t                            fileMinSize;
	uint64_t                            fileMaxSize;
	vm_offset_t                         videoMapping;
	vm_size_t                           videoAllocSize;
	vm_size_t                           videoMapSize;
	uint8_t *                           consoleMapping;
	uint8_t                             haveFastBoot;
	uint8_t                             saveBootAudioVolume;
	uint8_t                             hwEncrypt;
	uint8_t                             wiredCryptKey[kIOHibernateAESKeySize / 8];
	uint8_t                             cryptKey[kIOHibernateAESKeySize / 8];
	size_t                              volumeCryptKeySize;
	uint8_t                             volumeCryptKey[64];
};
typedef struct IOHibernateVars IOHibernateVars;

#endif          /* __cplusplus */

enum{
	kIOHibernateTagSignature = 0x53000000,
	kIOHibernateTagLength    = 0x00001fff,
};

#ifdef __cplusplus
extern "C"
#endif          /* __cplusplus */
uint32_t
hibernate_sum_page(uint8_t *buf, uint32_t ppnum);

extern vm_offset_t segHIBB;
extern unsigned long segSizeHIB;
extern vm_offset_t segDATAB;
extern unsigned long segSizeDATA;

extern ppnum_t gIOHibernateHandoffPages[];
extern uint32_t gIOHibernateHandoffPageCount;
