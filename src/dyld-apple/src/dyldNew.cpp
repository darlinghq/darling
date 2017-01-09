/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2004-2008 Apple Inc. All rights reserved.
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


#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <mach/mach.h>
#include <sys/mman.h>

extern "C" void* __dso_handle;

#include "dyld.h"
#include "dyldLibSystemInterface.h"

//
//	 dyld initially allocates all memory from a pool inside dyld.
//   Once libSystem.dylib is initialized, dyld uses libSystem's malloc/free.
//

#if __LP64__
	// room for about ~1000 initial dylibs
	#define DYLD_POOL_CHUNK_SIZE 200*1024
#else
	// room for about ~900 initial dylibs
	#define DYLD_POOL_CHUNK_SIZE 150*1024
#endif

struct dyld_static_pool {
	dyld_static_pool*	previousPool;
	uint8_t*			current;
	uint8_t*			end;
	uint8_t				pool[1]; 
};

// allocate initial pool independently of pool header to take less space on disk
static uint8_t initialPoolContent[DYLD_POOL_CHUNK_SIZE] __attribute__((__aligned__(16)));
static dyld_static_pool initialPool = { NULL, initialPoolContent, &initialPoolContent[DYLD_POOL_CHUNK_SIZE] };
static dyld_static_pool* currentPool = &initialPool;


void* malloc(size_t size)
{
	if ( (dyld::gLibSystemHelpers != NULL) && dyld::gProcessInfo->libSystemInitialized ) {
		void* p = dyld::gLibSystemHelpers->malloc(size);
		//dyld::log("malloc(%lu) => %p from libSystem\n", size, p);
		return p;
	}
	else {
		if ( size > DYLD_POOL_CHUNK_SIZE ) {
			dyld::log("dyld malloc overflow: size=%zu\n", size);
			exit(1);
		}
		size = (size+sizeof(void*)-1) & (-sizeof(void*)); // pointer align
		uint8_t* result = currentPool->current;
		currentPool->current += size;
		if ( currentPool->current > currentPool->end ) {
			vm_address_t addr = 0;
			kern_return_t r = vm_allocate(mach_task_self(), &addr, DYLD_POOL_CHUNK_SIZE, VM_FLAGS_ANYWHERE);
			if ( r != KERN_SUCCESS ) {
				dyld::log("out of address space for dyld memory pool\n");
				exit(1);
			}
			dyld_static_pool* newPool = (dyld_static_pool*)addr;
			newPool->previousPool = NULL;
			newPool->current = newPool->pool;
			newPool->end = (uint8_t*)(addr + DYLD_POOL_CHUNK_SIZE);
			newPool->previousPool = currentPool;
			currentPool = newPool;
			if ( (currentPool->current + size) > currentPool->end ) {
				dyld::log("dyld memory pool exhausted: size=%lu\n", size);
				exit(1);
			}
			result = currentPool->current;
			currentPool->current += size;
		}
		//dyld::log("%p = malloc(%3lu) from pool %p, free space = %lu\n", result, size, currentPool, (long)(currentPool->end - currentPool->current));
		return result;
	}
}


void free(void* ptr)
{
	// ignore any pointer within dyld (i.e. stuff from pool or static strings)
	if ( (dyld::gLibSystemHelpers != NULL) && ((ptr < &__dso_handle) || (ptr >= &initialPoolContent[DYLD_POOL_CHUNK_SIZE])) ) {
		// ignore stuff in any dynamically alloated dyld pools
		for (dyld_static_pool* p = currentPool; p != NULL; p = p->previousPool) {
			if ( (p->pool <= ptr) && (ptr < p->end) ) {
				// do nothing, pool entries can't be reclaimed
				//dyld::log("free(%p) from dynamic pool\n", ptr);
				return;
			}
		}
		
		//dyld::log("free(%p) from libSystem\n", ptr);
		return dyld::gLibSystemHelpers->free(ptr);
	}
	else {
		// do nothing, pool entries can't be reclaimed
		//dyld::log("free(%p) from static pool\n", ptr);
	}
}


void* calloc(size_t count, size_t size)
{
	if ( dyld::gLibSystemHelpers != NULL ) {
		void* result = dyld::gLibSystemHelpers->malloc(size*count);
		bzero(result, size*count);
		return result;
	}
	else {
		// Check for overflow of integer multiplication
		size_t total = count * size;
		if ( total/count != size ) {
			dyld::log("dyld calloc overflow: count=%zu, size=%zu\n", count, size);
			exit(1);
		}
		return malloc(total);
	}
}


void* realloc(void *ptr, size_t size)
{
	void* result = malloc(size);
	memcpy(result, ptr, size);
	return result;
}

//     void* reallocf(void *ptr, size_t size);
//     void* valloc(size_t size);

// needed __libc_init()
extern "C" int _malloc_lock;
int _malloc_lock = 0;


// <rdar://problem/12857033> dyld calls this which uses libSystem.dylib's vm_allocate if available
int vm_alloc(vm_address_t* addr, vm_size_t size, uint32_t flags)
{
	if ( (dyld::gLibSystemHelpers != NULL) && (dyld::gLibSystemHelpers->version >= 12) ) {
		return dyld::gLibSystemHelpers->vm_alloc(mach_task_self(), addr, size, flags);
	}
	else {
		return ::vm_allocate(mach_task_self(), addr, size, flags);
	}
}

void* xmmap(void* addr, size_t len, int prot, int flags, int fd, off_t offset)
{
	if ( (dyld::gLibSystemHelpers != NULL) && (dyld::gLibSystemHelpers->version >= 12) ) {
		return dyld::gLibSystemHelpers->mmap(addr, len, prot, flags, fd, offset);
	}
	else {
		return ::mmap(addr, len, prot, flags, fd, offset);
	}
}



