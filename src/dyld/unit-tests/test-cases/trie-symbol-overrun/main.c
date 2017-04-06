/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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
#include <mach-o/dyld.h>
#include <mach/mach.h>
#include <sys/mman.h>
#include <dlfcn.h>  
#include <Availability.h>  


#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()



int main()
{
	// allocate two pages
	vm_address_t addr = 0;
	kern_return_t r = vm_allocate(mach_task_self(), &addr, 8192, VM_FLAGS_ANYWHERE);
	if ( r != KERN_SUCCESS )  {
		FAIL("vm_allocate returned %d", r);
		exit(0);
	}
	// mark the second page unreadable
	mprotect((char*)(addr+4096), 4096, 0);
	// copy a string to the end of the first page
	char* sym = (char*)(addr+4096-5);
	strcpy(sym, "_abd");

	// call a dyld API that uses the string
	// if dyld reads past the end of the string, it will crash
	// <rdar://problem/6493245> trie parser can read past end of input symbol name
#if defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && (__MAC_OS_X_VERSION_MIN_REQUIRED <= __MAC_10_5)
	_dyld_lookup_and_bind(sym, NULL, NULL);
#else
	dlsym(RTLD_DEFAULT, sym);
#endif
	PASS("trie-symbol-overrun");
	return EXIT_SUCCESS;
}
