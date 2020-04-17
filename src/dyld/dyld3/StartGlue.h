/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __START_GLUE_H__
#define __START_GLUE_H__

// Implemented in start_glue.s
// Declare 'start' as a character, so that we can index into it.
// Avoid arithmetic on function pointers.
extern "C" char start;


// <rdar://problem/12792039> need 'start' to be one atom, but entry is in interior

#if __x86_64__ || __i386__ 
	#define address_of_start (void*)((uintptr_t)&start + 1)
#elif __arm64__
	#define address_of_start (void*)((uintptr_t)&start + 4)
#elif __arm__
	#define address_of_start (void*)((uintptr_t)&start + 2)
#endif



#endif // __START_GLUE_H__
