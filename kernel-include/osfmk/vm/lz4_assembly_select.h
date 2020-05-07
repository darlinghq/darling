/*
 * Copyright (c) 2016-2016 Apple Inc. All rights reserved.
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

#pragma once

//  Common header to enable/disable the assembly code paths
//  Rule: one define for each assembly source file

//  To enable assembly
#if defined __arm64__
#define LZ4_ENABLE_ASSEMBLY_ENCODE_ARM64 1
#define LZ4_ENABLE_ASSEMBLY_DECODE_ARM64 1
#elif defined __ARM_NEON__
#define LZ4_ENABLE_ASSEMBLY_ENCODE_ARMV7 1
#define LZ4_ENABLE_ASSEMBLY_DECODE_ARMV7 1
#elif defined __x86_64__
#define LZ4_ENABLE_ASSEMBLY_DECODE_X86_64 1
#endif

//  To disable C
#define LZ4_ENABLE_ASSEMBLY_ENCODE ((LZ4_ENABLE_ASSEMBLY_ENCODE_ARMV7) || (LZ4_ENABLE_ASSEMBLY_ENCODE_ARM64))
#define LZ4_ENABLE_ASSEMBLY_DECODE (LZ4_ENABLE_ASSEMBLY_DECODE_ARM64 || LZ4_ENABLE_ASSEMBLY_DECODE_ARMV7 || LZ4_ENABLE_ASSEMBLY_DECODE_X86_64)
