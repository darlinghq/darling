/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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
#include <unistd.h>
#include <Availability.h>
#include <mach-o/getsect.h>
#include <mach/mach.h> 
#include <mach/mach_vm.h> 
#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

#if __MAC_OS_X_VERSION_MIN_REQUIRED

extern void foo();
extern int fooData;

static vm_prot_t getPermission(void* addr)
{
    mach_vm_address_t				address = (mach_vm_address_t)(uintptr_t)addr;
    kern_return_t					result;
    mach_port_t						object_name;
    vm_region_basic_info_data_64_t	info;
    mach_msg_type_number_t			count;
    mach_vm_size_t					size = 4096;
	
	count = VM_REGION_BASIC_INFO_COUNT_64;
	result = mach_vm_region(mach_task_self(),
					&address,
					&size,
					VM_REGION_BASIC_INFO_64,
					(vm_region_info_t)&info,
					&count,
					&object_name);       
	//fprintf(stderr, "result=%X, info.protection=%X\n", result, info.protection);
	if ( result == KERN_SUCCESS ) 
		return info.protection;
	return 0;
}


static void* getStubAddr()
{
	unsigned long size;
#if __i386__
	return getsectdata("__IMPORT", "__jump_table", &size);
#elif __ppc__
	void* p = getsectdata("__TEXT", "__picsymbolstub1", &size);
	if ( p != NULL ) 
		return p;
	return getsectdata("__TEXT", "__symbol_stub1", &size);
#elif __ppc64__
	return getsectdata("__TEXT", "__picsymbolstub1", &size);
#elif __x86_64__
	return getsectdata("__TEXT", "__symbol_stub1", &size);
#elif __arm__
	void* p = getsectdata("__TEXT", "__symbol_stub4", &size);
	if ( p != NULL ) 
		return p;
	return getsectdata("__TEXT", "__symbolstub1", &size);
#else
	#error unknown arch
#endif
}


static void checkStubs(void* addr)
{
	vm_prot_t perm = getPermission(addr);
	if ( (perm == 0) || ((perm & VM_PROT_WRITE) != 0) ) {
		FAIL("read-only-stubs: bad permissions %d at address %p", perm, addr);
		exit(0);
	}
}


int main()
{
	void* stubAddr = getStubAddr();	
#if __i386__
	if ( stubAddr != NULL )
#endif
	{
	checkStubs(stubAddr);
	fooData = 1;
	foo();
	checkStubs(stubAddr);
	}
	PASS("read-only-stubs");
	return EXIT_SUCCESS;
}

#else

int main()
{
	// iOS does not have modifiable stubs
	PASS("read-only-stubs");
	return EXIT_SUCCESS;
}

#endif

