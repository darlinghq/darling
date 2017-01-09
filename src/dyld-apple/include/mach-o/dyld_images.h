/*
 * Copyright (c) 2006-2010 Apple Inc. All rights reserved.
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
#ifndef _DYLD_IMAGES_
#define _DYLD_IMAGES_

#include <stdbool.h>
#include <unistd.h>
#include <mach/mach.h>

#ifdef __cplusplus
extern "C" {
#endif



/* 
 *	Beginning in Mac OS X 10.4, this is how gdb discovers which mach-o images are loaded in a process.
 *
 *	gdb looks for the symbol "_dyld_all_image_infos" in dyld.  It contains the fields below.  
 *
 *	For a snashot of what images are currently loaded, the infoArray fields contain a pointer
 *	to an array of all images. If infoArray is NULL, it means it is being modified, come back later.
 *
 *	To be notified of changes, gdb sets a break point on the address pointed to by the notificationn
 *	field.  The function it points to is called by dyld with an array of information about what images 
 *	have been added (dyld_image_adding) or are about to be removed (dyld_image_removing). 
 *
 * The notification is called after infoArray is updated.  This means that if gdb attaches to a process
 * and infoArray is NULL, gdb can set a break point on notification and let the proccess continue to
 * run until the break point.  Then gdb can inspect the full infoArray.
 *
 * The dyldVersion field always points to a C string that contains the dyld version.  For instance,
 * in dyld-127.3, dyldVersion would contain a pointer to "127.3".
 *
 * The errorMessage and terminationFlags fields are normally zero.  If dyld terminates a process
 * (for instance because a required dylib or symbol is missing), then the errorMessage field will
 * be set to point to a C string message buffer containing the reason dyld terminate the process.
 * The low bit of the terminationFlags will be set if dyld terminated the process before any user
 * code ran, in which case there is no need for the crash log to contain the backtrace.
 *
 * When dyld terminates a process because some required dylib or symbol cannot be bound, in 
 * addition to the errorMessage field, it now sets the errorKind field and the corresponding
 * fields: errorClientOfDylibPath, errorTargetDylibPath, errorSymbol.
 *
 */

enum dyld_image_mode { dyld_image_adding=0, dyld_image_removing=1, dyld_image_info_change=2 };

struct dyld_image_info {
	const struct mach_header*	imageLoadAddress;	/* base address image is mapped into */
	const char*					imageFilePath;		/* path dyld used to load the image */
	uintptr_t					imageFileModDate;	/* time_t of image file */
													/* if stat().st_mtime of imageFilePath does not match imageFileModDate, */
													/* then file has been modified since dyld loaded it */
};

struct dyld_uuid_info {
	const struct mach_header*	imageLoadAddress;	/* base address image is mapped into */
	uuid_t						imageUUID;			/* UUID of image */
};

typedef void (*dyld_image_notifier)(enum dyld_image_mode mode, uint32_t infoCount, const struct dyld_image_info info[]);

/* for use in dyld_all_image_infos.errorKind field */
enum {	dyld_error_kind_none=0, 
		dyld_error_kind_dylib_missing=1, 
		dyld_error_kind_dylib_wrong_arch=2,
		dyld_error_kind_dylib_version=3,
		dyld_error_kind_symbol_missing=4
	};

/* internal limit */ 
#define DYLD_MAX_PROCESS_INFO_NOTIFY_COUNT  8

struct dyld_all_image_infos {
	uint32_t						version;		/* 1 in Mac OS X 10.4 and 10.5 */
	uint32_t						infoArrayCount;
	const struct dyld_image_info*	infoArray;
	dyld_image_notifier				notification;		
	bool							processDetachedFromSharedRegion;
	/* the following fields are only in version 2 (Mac OS X 10.6, iPhoneOS 2.0) and later */
	bool							libSystemInitialized;
	const struct mach_header*		dyldImageLoadAddress;
	/* the following field is only in version 3 (Mac OS X 10.6, iPhoneOS 3.0) and later */
	void*							jitInfo;
	/* the following fields are only in version 5 (Mac OS X 10.6, iPhoneOS 3.0) and later */
	const char*						dyldVersion;
	const char*						errorMessage;
	uintptr_t						terminationFlags;
	/* the following field is only in version 6 (Mac OS X 10.6, iPhoneOS 3.1) and later */
	void*							coreSymbolicationShmPage;
	/* the following field is only in version 7 (Mac OS X 10.6, iPhoneOS 3.1) and later */
	uintptr_t						systemOrderFlag;
	/* the following field is only in version 8 (Mac OS X 10.7, iPhoneOS 3.1) and later */
	uintptr_t						uuidArrayCount;
	const struct dyld_uuid_info*	uuidArray;		/* only images not in dyld shared cache */
	/* the following field is only in version 9 (Mac OS X 10.7, iOS 4.0) and later */
	struct dyld_all_image_infos*	dyldAllImageInfosAddress;
	/* the following field is only in version 10 (Mac OS X 10.7, iOS 4.2) and later */
	uintptr_t						initialImageCount;
	/* the following field is only in version 11 (Mac OS X 10.7, iOS 4.2) and later */
	uintptr_t						errorKind;
	const char*						errorClientOfDylibPath;
	const char*						errorTargetDylibPath;
	const char*						errorSymbol;
	/* the following field is only in version 12 (Mac OS X 10.7, iOS 4.3) and later */
	uintptr_t						sharedCacheSlide;
	/* the following field is only in version 13 (Mac OS X 10.9, iOS 7.0) and later */
	uint8_t							sharedCacheUUID[16];
	/* the following field is only in version 15 (macOS 10.12, iOS 10.0) and later */
	uintptr_t						sharedCacheBaseAddress;
	uint64_t						infoArrayChangeTimestamp;
	const char*						dyldPath;
	mach_port_t						notifyPorts[DYLD_MAX_PROCESS_INFO_NOTIFY_COUNT];
#if __LP64__
	uintptr_t						reserved[13-(DYLD_MAX_PROCESS_INFO_NOTIFY_COUNT/2)];
#else
	uintptr_t						reserved[12-DYLD_MAX_PROCESS_INFO_NOTIFY_COUNT];
#endif
};


/*
 * Beginning in Mac OS X 10.5, this is how gdb discovers where the shared cache is in a process.
 * Images that are in the shared cache have their segments rearranged, so when using imageFilePath
 * to load the file from disk, you have to know to adjust addresses based on how their segment
 * was rearranged.
 *
 * gdb looks for the symbol "_dyld_shared_region_ranges" in dyld. 
 * 
 * It contains information the count of shared regions used by the process.  The count is
 * the number of start/length pairs.  
 */
struct dyld_shared_cache_ranges {
	uintptr_t					sharedRegionsCount;	/* how many ranges follow */
	struct {
		uintptr_t	start;
		uintptr_t	length;
	}							ranges[4];			/* max regions */
};
extern struct dyld_shared_cache_ranges dyld_shared_cache_ranges;



#ifdef __cplusplus
}
#endif

#endif /* _DYLD_IMAGES_ */
