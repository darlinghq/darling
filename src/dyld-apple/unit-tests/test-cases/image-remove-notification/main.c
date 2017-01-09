/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
#include <stdbool.h>
#include <mach-o/dyld.h>  
#include <dlfcn.h>  

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


static void* sExpectedBundleBaseAddress = 0;
static bool sNotified = false;

static void removeNotification(const struct mach_header *mh, intptr_t vmaddr_slide)
{
	if ( mh == sExpectedBundleBaseAddress )
		sNotified = true;
}



int main(int argc, const char* argv[])
{
	// tell dyld we want to know when images go away
	_dyld_register_func_for_remove_image(removeNotification);
	
	void* handle = dlopen("foo.bundle", RTLD_LAZY);
	if ( handle == NULL ) {
		FAIL("image-remove-notification: dlopen(foo.bundle) failed: %s", dlerror());
		exit(0);
	}
	
	void* sym = dlsym(handle, "foo");
	if ( sym == NULL ) {
		FAIL("image-remove-notification: dlsym(handle, \"foo\") failed: %s", dlerror());
		exit(0);
	}

	Dl_info info;
	if ( dladdr(sym, &info) == 0 ) {
		FAIL("image-remove-notification: dladdr() failed");
		exit(0);
	}
	
	// record the base address we expect to get notification about
	sExpectedBundleBaseAddress = info.dli_fbase;
	
	int result = dlclose(handle);
	if ( result != 0 ) {
		FAIL("image-remove-notification: dlclose(handle) returned %d, %s", result, dlerror());
		exit(0);
	}

	if ( sNotified )
		PASS("image-remove-notification");
	else
		FAIL("image-remove-notification");
		
	return EXIT_SUCCESS;
}
