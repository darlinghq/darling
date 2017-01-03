/*
 * Copyright (c) 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
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

#include <machine/cpu_capabilities.h>
#include "platfunc.h"

PLATFUNC_DESCRIPTOR_PROTOTYPE(memcpy, sse42)
PLATFUNC_DESCRIPTOR_PROTOTYPE(memcpy, sse3x)

static const platfunc_descriptor *memcpy_platfunc_descriptors[] = {
	PLATFUNC_DESCRIPTOR_REFERENCE(memcpy, sse42),
	PLATFUNC_DESCRIPTOR_REFERENCE(memcpy, sse3x),
	0
};

void *memcpy_chooser() __asm__("_memcpy");
void *memcpy_chooser() {
	__asm__(".desc _memcpy, 0x100");
	return find_platform_function((const platfunc_descriptor **) memcpy_platfunc_descriptors);
}
