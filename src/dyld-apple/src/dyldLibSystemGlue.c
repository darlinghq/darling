/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2009-2010 Apple Inc. All rights reserved.
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

#include <Availability.h>
#include <stddef.h>
#include <TargetConditionals.h>

 
//
// This is the temporary private interface between libSystem.B.dylib and dyld
//


int          NXArgc = 0;
const char** NXArgv = NULL;
const char** environ = NULL;
const char*  __progname = NULL;



//
// Long ago, the compiler driver added -ldylib1.o to every dylib which caused a
// __DATA,__dyld section to be added every dylib.  The new LINKEDIT format no longer requires
// images to have a __DATA,__dyld section.  But until libdyld.dylib and dyld update
// to some sort of vtable based interface, libdyld still needs a __DATA,__dyld section.
// The code below adds that section.
//
struct __DATA__dyld { 
	long			lazy; 
	int				(*lookup)(const char*, void**);
	// ProgramVars
	const void*		mh;
	int*			NXArgcPtr;
	const char***	NXArgvPtr;
	const char***	environPtr;
	const char**	__prognamePtr;
};

static volatile struct __DATA__dyld  myDyldSection __attribute__ ((section ("__DATA,__dyld"))) 
	= { 0, 0, NULL, &NXArgc, &NXArgv, &environ, &__progname };

#if __arm__ && __MAC_OS_X_VERSION_MIN_REQUIRED
// <rdar://problem/8755380>
// For historical reasons, gcc and llvm-gcc added -ldylib1.o to the link line of armv6 
// dylibs when targeting MacOSX (but not iOS).  clang cleans up that mistake, but doing
// so would break the libdyld build.  Making _dyld_func_lookup weak,hidden means if
// dylib1.o is used, it overrides this, otherwise this implementation is used.
__attribute__((weak))
#endif
__attribute__((visibility("hidden")))
int _dyld_func_lookup(const char* dyld_func_name, void **address)
{
	return (*myDyldSection.lookup)(dyld_func_name, address);
}

