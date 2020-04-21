/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2004-2009 Apple Inc. All rights reserved.
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

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <mach-o/loader.h>
#include <mach-o/dyld_images.h>
#include <mach-o/dyld_process_info.h>

#include <vector>

#include "Tracing.h"
#include "ImageLoader.h"
#include "dyld2.h"

extern "C" 	void _dyld_debugger_notification(enum dyld_notify_mode mode, unsigned long count, uint64_t machHeaders[]);

#if __IPHONE_OS_VERSION_MIN_REQUIRED
	#define INITIAL_UUID_IMAGE_COUNT 4
#else
	#define INITIAL_UUID_IMAGE_COUNT 32
#endif

VECTOR_NEVER_DESTRUCTED(dyld_image_info);
VECTOR_NEVER_DESTRUCTED(dyld_uuid_info);

static std::vector<dyld_image_info> sImageInfos;
static std::vector<dyld_uuid_info>  sImageUUIDs;

size_t allImagesCount()
{
	return sImageInfos.size();
}

const mach_header* allImagesIndexedMachHeader(uint32_t index)
{
	if ( index < sImageInfos.size() )
		return sImageInfos[index].imageLoadAddress;
	else
		return NULL;
}

const char* allImagesIndexedPath(uint32_t index)
{
	if ( index < sImageInfos.size() )
		return sImageInfos[index].imageFilePath;
	else
		return NULL;
}


void addImagesToAllImages(uint32_t infoCount, const dyld_image_info info[])
{
	// make initial size large enough that we probably won't need to re-alloc it
	if ( sImageInfos.size() == 0 )
		sImageInfos.reserve(INITIAL_IMAGE_COUNT);
	if ( sImageUUIDs.capacity() == 0 )
		sImageUUIDs.reserve(4);
	// set infoArray to NULL to denote it is in-use
	dyld::gProcessInfo->infoArray = NULL;
	
	// append all new images
	for (uint32_t i=0; i < infoCount; ++i)
		sImageInfos.push_back(info[i]);
	dyld::gProcessInfo->infoArrayCount = (uint32_t)sImageInfos.size();
	dyld::gProcessInfo->infoArrayChangeTimestamp = mach_absolute_time();

	// set infoArray back to base address of vector (other process can now read)
	dyld::gProcessInfo->infoArray = &sImageInfos[0];
}

#if TARGET_OS_SIMULATOR
// called once in dyld_sim start up to copy image list from host dyld to sImageInfos
void syncProcessInfo()
{
	// may want to set version field of gProcessInfo if it might be different than host
	if ( sImageInfos.size() == 0 ) {
		sImageInfos.reserve(INITIAL_IMAGE_COUNT);
		if ( dyld::gProcessInfo->infoArray != NULL ) {
			for (uint32_t i=0; i < dyld::gProcessInfo->infoArrayCount; ++i) {
				sImageInfos.push_back(dyld::gProcessInfo->infoArray[i]);
			}
			dyld::gProcessInfo->infoArray = &sImageInfos[0];
			dyld::gProcessInfo->infoArrayCount = (uint32_t)sImageInfos.size();
		}
	}
	dyld::gProcessInfo->notification(dyld_image_info_change, 0, NULL);
}
#endif

const char* notifyGDB(enum dyld_image_states state, uint32_t infoCount, const dyld_image_info info[])
{
	// tell gdb that about the new images
	uint64_t t0 = mach_absolute_time();
	dyld::gProcessInfo->notification(dyld_image_adding, infoCount, info);
	uint64_t t1 = mach_absolute_time();
	ImageLoader::fgTotalDebuggerPausedTime += (t1-t0);

	// <rdar://problem/7739489> record initial count of images  
	// so CrashReporter can note which images were dynamically loaded
	if ( dyld::gProcessInfo->initialImageCount == 0 )
		dyld::gProcessInfo->initialImageCount = dyld::gProcessInfo->infoArrayCount;
	return NULL;
}



void addNonSharedCacheImageUUID(const dyld_uuid_info& info)
{
	// set uuidArray to NULL to denote it is in-use
	dyld::gProcessInfo->uuidArray = NULL;
	
	// append all new images
	sImageUUIDs.push_back(info);
	dyld::gProcessInfo->uuidArrayCount = sImageUUIDs.size();
	
	// set uuidArray back to base address of vector (other process can now read)
	dyld::gProcessInfo->uuidArray = &sImageUUIDs[0];
}

void removeImageFromAllImages(const struct mach_header* loadAddress)
{
	dyld_image_info goingAway;
	
	// set infoArray to NULL to denote it is in-use
	dyld::gProcessInfo->infoArray = NULL;
	
	// remove image from infoArray
	for (std::vector<dyld_image_info>::iterator it=sImageInfos.begin(); it != sImageInfos.end(); it++) {
		if ( it->imageLoadAddress == loadAddress ) {
			goingAway = *it;
			sImageInfos.erase(it);
			break;
		}
	}
	dyld::gProcessInfo->infoArrayCount = (uint32_t)sImageInfos.size();
	
	// set infoArray back to base address of vector
	dyld::gProcessInfo->infoArray = &sImageInfos[0];


	// set uuidArrayCount to NULL to denote it is in-use
	dyld::gProcessInfo->uuidArray = NULL;
	
	// remove image from infoArray
	for (std::vector<dyld_uuid_info>::iterator it=sImageUUIDs.begin(); it != sImageUUIDs.end(); it++) {
		if ( it->imageLoadAddress == loadAddress ) {
			sImageUUIDs.erase(it);
			break;
		}
	}
	dyld::gProcessInfo->uuidArrayCount = sImageUUIDs.size();
	dyld::gProcessInfo->infoArrayChangeTimestamp = mach_absolute_time();

	// set infoArray back to base address of vector
	dyld::gProcessInfo->uuidArray = &sImageUUIDs[0];

	// tell gdb that about the new images
	dyld::gProcessInfo->notification(dyld_image_removing, 1, &goingAway);
}


#if TARGET_OS_SIMULATOR
	namespace dyld {
		struct dyld_all_image_infos* gProcessInfo = NULL;
	}
#else

	static void gdb_image_notifier(enum dyld_image_mode mode, uint32_t infoCount, const dyld_image_info info[])
	{
		dyld3::ScopedTimer(DBG_DYLD_GDB_IMAGE_NOTIFIER, 0, 0, 0);
		uint64_t machHeaders[infoCount];
		for (uint32_t i=0; i < infoCount; ++i) {
			machHeaders[i] = (uintptr_t)(info[i].imageLoadAddress);
		}
		switch ( mode ) {
			 case dyld_image_adding:
				_dyld_debugger_notification(dyld_notify_adding, infoCount, machHeaders);
				break;
			 case dyld_image_removing:
				_dyld_debugger_notification(dyld_notify_removing, infoCount, machHeaders);
				break;
			default:
				break;
		}
		// do nothing
		// gdb sets a break point here to catch notifications
		//dyld::log("dyld: gdb_image_notifier(%s, %d, ...)\n", mode ? "dyld_image_removing" : "dyld_image_adding", infoCount);
		//for (uint32_t i=0; i < infoCount; ++i)
		//	dyld::log("dyld: %d loading at %p %s\n", i, info[i].imageLoadAddress, info[i].imageFilePath);
		//for (uint32_t i=0; i < dyld::gProcessInfo->infoArrayCount; ++i)
		//	dyld::log("dyld: %d loading at %p %s\n", i, dyld::gProcessInfo->infoArray[i].imageLoadAddress, dyld::gProcessInfo->infoArray[i].imageFilePath);
	}

	// only used with accelerator tables and ASan which images need to be re-loaded
	void resetAllImages()
	{
		sImageInfos.clear();
		sImageUUIDs.clear();
		_dyld_debugger_notification(dyld_notify_remove_all, 0, NULL);
	}

	extern void* __dso_handle;
	#define STR(s) # s
	#define XSTR(s) STR(s)

	struct dyld_all_image_infos  dyld_all_image_infos __attribute__ ((section ("__DATA,__all_image_info"))) 
								= { 
									16, 0, {NULL}, &gdb_image_notifier, false, false, (const mach_header*)&__dso_handle, NULL,
									XSTR(DYLD_VERSION), NULL, 0, NULL, 0, 0, NULL, &dyld_all_image_infos,
									0, 0, NULL, NULL, NULL, 0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
									0, {0}, "/usr/lib/dyld", {0}, {0}, 0
									};

	struct dyld_shared_cache_ranges dyld_shared_cache_ranges;

	namespace dyld {
		struct dyld_all_image_infos* gProcessInfo = &dyld_all_image_infos;
	}
#endif
 
