/*
 * Copyright (c) 2015 Apple Computer, Inc. All rights reserved.
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
#include <mach-o/dyld.h> 
#include <mach-o/dyld_priv.h> 

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


extern void foo();


// checks dladdr() and dyld_image_header_containing_address() return same image
static void verify(void* addr)
{
	Dl_info info;
	if ( dladdr(addr, &info) == 0 ) {
		FAIL("dladdr(%p, xx) failed", addr);
		exit(0);
	}
	const struct mach_header* mh = dyld_image_header_containing_address(addr);
	if ( mh != info.dli_fbase ) {
		FAIL("dladdr's  dli_fbase != dyld_image_header_containing_address()");
		exit(0);
	}
}


int main()
{
	verify(&main);
	verify(&foo);
	
	int x;
	if ( dyld_image_path_containing_address(&x) != NULL ) {
		FAIL("dyld_image_header_containing_address() of stack variable not NULL");
		exit(0);
	}
  
	PASS("dyld_image_header_containing_address");
	return EXIT_SUCCESS;
}
