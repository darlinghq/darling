/*
 * Copyright (c) 2007-2010 Apple Inc. All rights reserved.
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
#include <mach-o/ldsyms.h>
#include <mach/mach.h> 
#include <mach/mach_vm.h> 

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

#if __MAC_OS_X_VERSION_MIN_REQUIRED

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
	if ( result == KERN_SUCCESS ) 
		return info.protection;
	return 0;
}


static void* getStubAddr()
{
#if __LP64__
	uint64_t size;
#else
	uint32_t size;
#endif
	uintptr_t slide = (uintptr_t)&_mh_dylib_header; // assume dylib is zero-base so slide == load address
#if __i386__
	return getsectdatafromheader(&_mh_dylib_header, "__IMPORT", "__symbol_stub", &size) + slide;
#elif __ppc__
	return getsectdatafromheader(&_mh_dylib_header, "TEXT", "__picsymbolstub1", &size) + slide;
#elif __ppc64__
	return getsectdatafromheader_64(&_mh_dylib_header, "__TEXT", "__picsymbolstub1", &size) + slide;
#elif __x86_64__
	return getsectdatafromheader_64(&_mh_dylib_header, "__TEXT", "__symbol_stub1", &size) + slide;
#elif __arm__
	void* p = getsectdata("__TEXT", "__picsymbolstub4", (unsigned long*)&size);
	if ( p != NULL ) 
		return getsectdatafromheader(&_mh_dylib_header, "__TEXT", "__picsymbolstub4", &size) + slide;
	return getsectdatafromheader(&_mh_dylib_header, "__TEXT", "__symbolstub1", &size) + slide;
#else
	#error unknown arch
#endif
}


static void checkStubs(void* addr)
{
	vm_prot_t perm = getPermission(addr);
	//fprintf(stderr, "perm=0x%02X\n", perm);
	if ( (perm == 0) || ((perm & VM_PROT_EXECUTE) == 0) ) {
		FAIL("text-reloc-perms: missing exec permission 0x%0X at address %p", perm, addr);
		exit(0);
	}
}


void foo()
{
	void* stubAddr = getStubAddr();	
	checkStubs(stubAddr);
}

#else

void foo() 
{
	// iOS does not have text relocs
}

#endif

