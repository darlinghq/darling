/*
 * Copyright (c) 2005-2009 Apple Inc. All rights reserved.
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
#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <string.h>
#include <dlfcn.h>
#include <mach/mach.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


///
/// This tests that dlclose() actually unmmaps the image
///

static void trySO(const char* path)
{	
	void* handle = dlopen(path, RTLD_LAZY);
	if ( handle == NULL ) {
		const char* msg = dlerror();
		FAIL("dlopen(\"%s\" RTLD_LAZY) failed but it should have worked: %s", path, msg);
		exit(0);
	}
	void* sym = dlsym(handle, "foo");
	if ( sym == NULL ) {
		const char* msg = dlerror();
		FAIL("dlsym(handle, \"foo\") failed but it should have worked: %s", msg);
		exit(0);
	}
	
	int result = dlclose(handle);
	if ( result != 0 ) {
		FAIL("dlclose(handle) returned %d", result);
		exit(0);
	}	
	
	// now try to create a page where foo() was
	vm_address_t addr = ((uintptr_t)sym) & (-4096);
	kern_return_t r = vm_allocate(mach_task_self(), &addr, 4096, VM_FLAGS_FIXED);
	if ( r != KERN_SUCCESS )  {
		FAIL("dlclose-unmap: could not allocate page where SO was previously mapped", result);
		exit(0);
	}
}


int main()
{
	trySO("test.bundle");
	trySO("test.dylib");
	
	PASS("dlclose-unmap");
	return 0;
}
