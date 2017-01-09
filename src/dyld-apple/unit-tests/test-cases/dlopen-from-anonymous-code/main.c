/*
 * Copyright (c) 2006-2007 Apple Inc. All rights reserved.
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
#include <string.h> // memcpy
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <dlfcn.h>
#include <libkern/OSCacheControl.h> // sys_icache_invalidate
#include <sys/mman.h> // for mprotext
#include <mach/mach.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()



void* calldlopen(const char* path, int mode, void* (*dlopen_proc)(const char* path, int mode))
{
	return (*dlopen_proc)(path, mode);
}

#if __thumb__
	#define START_OF_FUNC(x) ((void*)((long)x & (-2)))
	#define ADDR_FROM_BLOCK(x) ((void*)((long)x | 1))
#else
	#define START_OF_FUNC(x) (x)
	#define ADDR_FROM_BLOCK(x) (x)
#endif

//
// try calling dlopen() from code not owned by dyld
//
int main()
{
	// now try to create a page where foo() was
	vm_address_t addr = 0;
	kern_return_t r = vm_allocate(mach_task_self(), &addr, 4096, VM_FLAGS_ANYWHERE);
	if ( r != KERN_SUCCESS )  {
		FAIL("vm_allocate returned %d", r);
		return 0;
	}
	void* codeBlock = (void*)(addr);
	memcpy(codeBlock, START_OF_FUNC(calldlopen), 4096);
	sys_icache_invalidate(codeBlock, 4096);
	mprotect(codeBlock, 4096, PROT_READ | PROT_EXEC);
	//fprintf(stderr, "codeBlock=%p\n", codeBlock);
	
	void* (*caller)(const char* path, int mode, void* (*dlopen_proc)(const char* path, int mode)) = ADDR_FROM_BLOCK(codeBlock);
	
	void* handle = (*caller)("foo.bundle", RTLD_LAZY, &dlopen);
	if ( handle == NULL ) {
		FAIL("dlopen(\"%s\") failed with: %s", "foo.bundle", dlerror());
		exit(0);
	}
	
	void* sym = dlsym(handle, "foo");
	if ( sym == NULL ) {
		FAIL("dlsym(handle, \"foo\") failed");
		exit(0);
	}
	
	int result = dlclose(handle);
	if ( result != 0 ) {
		FAIL("dlclose(handle) returned %d", result);
		exit(0);
	}
  
	PASS("dlopen-from-anonymous-code");
	return EXIT_SUCCESS;
}
