/*
 * Copyright (c) 2014 Apple Computer, Inc. All rights reserved.
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

#ifndef _OS_OSSERIALIZEBINARY_H
#define _OS_OSSERIALIZEBINARY_H

enum{
	kOSSerializeDictionary   = 0x01000000U,
	kOSSerializeArray        = 0x02000000U,
	kOSSerializeSet          = 0x03000000U,
	kOSSerializeNumber       = 0x04000000U,
	kOSSerializeSymbol       = 0x08000000U,
	kOSSerializeString       = 0x09000000U,
	kOSSerializeData         = 0x0a000000U,
	kOSSerializeBoolean      = 0x0b000000U,
	kOSSerializeObject       = 0x0c000000U,
	kOSSerializeTypeMask     = 0x7F000000U,
	kOSSerializeDataMask     = 0x00FFFFFFU,
	kOSSerializeEndCollecton = 0x80000000U,
};

#define kOSSerializeBinarySignature        "\323\0\0"
#define kOSSerializeIndexedBinarySignature 0x000000D4

#endif /* _OS_OSSERIALIZEBINARY_H */
