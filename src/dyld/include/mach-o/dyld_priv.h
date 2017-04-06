/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2003-2010 Apple Inc. All rights reserved.
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
#ifndef _MACH_O_DYLD_PRIV_H_
#define _MACH_O_DYLD_PRIV_H_

#include <stdbool.h>
#include <Availability.h>
#include <mach-o/dyld.h>
#include <mach-o/dyld_images.h>

#if __cplusplus
extern "C" {
#endif /* __cplusplus */



//
// private interface between libSystem.dylib and dyld
//
extern int _dyld_func_lookup(const char* dyld_func_name, void **address);

//
// private interface between libSystem.dylib and dyld
//
extern void _dyld_fork_child();


// DEPRECATED
enum dyld_image_states
{
	dyld_image_state_mapped					= 10,		// No batch notification for this
	dyld_image_state_dependents_mapped		= 20,		// Only batch notification for this
	dyld_image_state_rebased				= 30, 
	dyld_image_state_bound					= 40,
	dyld_image_state_dependents_initialized	= 45,		// Only single notification for this
	dyld_image_state_initialized			= 50,
	dyld_image_state_terminated				= 60		// Only single notification for this
};

// DEPRECATED
typedef const char* (*dyld_image_state_change_handler)(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[]);



typedef void (*_dyld_objc_notify_mapped)(unsigned count, const char* const paths[], const struct mach_header* const mh[]);
typedef void (*_dyld_objc_notify_init)(const char* path, const struct mach_header* mh);
typedef void (*_dyld_objc_notify_unmapped)(const char* path, const struct mach_header* mh);


//
// Note: only for use by objc runtime
// Register handlers to be called when objc images are mapped, unmapped, and initialized.
// Dyld will call back the "mapped" function with an array of images that contain an objc-image-info section.
// Those images that are dylibs will have the ref-counts automatically bumped, so objc will no longer need to
// call dlopen() on them to keep them from being unloaded.  During the call to _dyld_objc_notify_register(),
// dyld will call the "mapped" function with already loaded objc images.  During any later dlopen() call,
// dyld will also call the "mapped" function.  Dyld will call the "init" function when dyld would be called
// initializers in that image.  This is when objc calls any +load methods in that image.
//
void _dyld_objc_notify_register(_dyld_objc_notify_mapped    mapped,
                                _dyld_objc_notify_init      init,
                                _dyld_objc_notify_unmapped  unmapped);



//
// Possible thread-local variable state changes for which you can register to be notified
//
enum dyld_tlv_states {
    dyld_tlv_state_allocated = 10,   // TLV range newly allocated
    dyld_tlv_state_deallocated = 20  // TLV range about to be deallocated
};

// 
// Info about thread-local variable storage.
// 
typedef struct {
    size_t info_size;    // sizeof(dyld_tlv_info)
    void * tlv_addr;     // Base address of TLV storage
    size_t tlv_size;     // Byte size of TLV storage
} dyld_tlv_info;

#if __BLOCKS__

// 
// Callback that notes changes to thread-local variable storage.
// 
typedef void (^dyld_tlv_state_change_handler)(enum dyld_tlv_states state, const dyld_tlv_info *info);

//
// Register a handler to be called when a thread adds or removes storage for thread-local variables.
// The registered handler will only be called from and on behalf of the thread that owns the storage.
// The registered handler will NOT be called for any storage that was 
//   already allocated before dyld_register_tlv_state_change_handler() was 
//   called. Use dyld_enumerate_tlv_storage() to get that information.
// Exists in Mac OS X 10.7 and later 
// 
extern void 
dyld_register_tlv_state_change_handler(enum dyld_tlv_states state, dyld_tlv_state_change_handler handler);

// 
// Enumerate the current thread-local variable storage allocated for the current thread. 
// Exists in Mac OS X 10.7 and later 
//
extern void 
dyld_enumerate_tlv_storage(dyld_tlv_state_change_handler handler);

#endif


//
// get slide for a given loaded mach_header  
// Mac OS X 10.6 and later
//
extern intptr_t _dyld_get_image_slide(const struct mach_header* mh);


//
// get pointer to this process's dyld_all_image_infos
// Exists in Mac OS X 10.4 and later through _dyld_func_lookup()
// Exists in Mac OS X 10.6 and later through libSystem.dylib
//
const struct dyld_all_image_infos* _dyld_get_all_image_infos() __attribute__((deprecated));



struct dyld_unwind_sections
{
	const struct mach_header*		mh;
	const void*						dwarf_section;
	uintptr_t						dwarf_section_length;
	const void*						compact_unwind_section;
	uintptr_t						compact_unwind_section_length;
};


//
// Returns true iff some loaded mach-o image contains "addr".
//	info->mh							mach header of image containing addr
//  info->dwarf_section					pointer to start of __TEXT/__eh_frame section
//  info->dwarf_section_length			length of __TEXT/__eh_frame section
//  info->compact_unwind_section		pointer to start of __TEXT/__unwind_info section
//  info->compact_unwind_section_length	length of __TEXT/__unwind_info section
//
// Exists in Mac OS X 10.6 and later 
extern bool _dyld_find_unwind_sections(void* addr, struct dyld_unwind_sections* info);


//
// This is an optimized form of dladdr() that only returns the dli_fname field.
//
// Exists in Mac OS X 10.6 and later 
extern const char* dyld_image_path_containing_address(const void* addr);


//
// This is an optimized form of dladdr() that only returns the dli_fbase field.
// Return NULL, if address is not in any image tracked by dyld.
//
// Exists in Mac OS X 10.11 and later
extern const struct mach_header* dyld_image_header_containing_address(const void* addr);



// Convienence constants for return values from dyld_get_sdk_version() and friends.
#define DYLD_MACOSX_VERSION_10_4		0x000A0400
#define DYLD_MACOSX_VERSION_10_5		0x000A0500
#define DYLD_MACOSX_VERSION_10_6		0x000A0600
#define DYLD_MACOSX_VERSION_10_7		0x000A0700
#define DYLD_MACOSX_VERSION_10_8		0x000A0800
#define DYLD_MACOSX_VERSION_10_9		0x000A0900
#define DYLD_MACOSX_VERSION_10_10		0x000A0A00
#define DYLD_MACOSX_VERSION_10_11		0x000A0B00
#define DYLD_MACOSX_VERSION_10_12		0x000A0C00

#define DYLD_IOS_VERSION_2_0		0x00020000
#define DYLD_IOS_VERSION_2_1		0x00020100
#define DYLD_IOS_VERSION_2_2		0x00020200
#define DYLD_IOS_VERSION_3_0		0x00030000
#define DYLD_IOS_VERSION_3_1		0x00030100
#define DYLD_IOS_VERSION_3_2		0x00030200
#define DYLD_IOS_VERSION_4_0		0x00040000
#define DYLD_IOS_VERSION_4_1		0x00040100
#define DYLD_IOS_VERSION_4_2		0x00040200
#define DYLD_IOS_VERSION_4_3		0x00040300
#define DYLD_IOS_VERSION_5_0		0x00050000
#define DYLD_IOS_VERSION_5_1		0x00050100
#define DYLD_IOS_VERSION_6_0		0x00060000
#define DYLD_IOS_VERSION_6_1		0x00060100
#define DYLD_IOS_VERSION_7_0		0x00070000
#define DYLD_IOS_VERSION_7_1		0x00070100
#define DYLD_IOS_VERSION_8_0		0x00080000
#define DYLD_IOS_VERSION_8_1		0x00080100
#define DYLD_IOS_VERSION_8_2		0x00080200
#define DYLD_IOS_VERSION_8_3		0x00080300
#define DYLD_IOS_VERSION_8_4		0x00080400
#define DYLD_IOS_VERSION_9_0		0x00090000
#define DYLD_IOS_VERSION_9_1		0x00090100
#define DYLD_IOS_VERSION_9_2		0x00090200
#define DYLD_IOS_VERSION_9_3		0x00090300
#define DYLD_IOS_VERSION_10_0		0x000A0000


#define DYLD_WATCHOS_VERSION_1_0	0x00010000
#define DYLD_WATCHOS_VERSION_2_0	0x00020000
#define DYLD_WATCHOS_VERSION_2_1	0x00020100
#define DYLD_WATCHOS_VERSION_2_2	0x00020200
#define DYLD_WATCHOS_VERSION_3_0	0x00030000



//
// This finds the SDK version a binary was built against.
// Returns zero on error, or if SDK version could not be determined.
//
// Exists in Mac OS X 10.8 and later 
// Exists in iOS 6.0 and later
extern uint32_t dyld_get_sdk_version(const struct mach_header* mh);


//
// This finds the SDK version that the main executable was built against.
// Returns zero on error, or if SDK version could not be determined.
//
// Note on WatchOS, this returns the equivalent iOS SDK version number
// (i.e an app built against WatchOS 2.0 SDK returne 9.0).  To see the
// platform specific sdk version use dyld_get_program_sdk_watch_os_version().
//
// Exists in Mac OS X 10.8 and later 
// Exists in iOS 6.0 and later
extern uint32_t dyld_get_program_sdk_version();


// Watch OS only.
// This finds the Watch OS SDK version that the main executable was built against.
// Exists in Watch OS 2.0 and later
extern uint32_t dyld_get_program_sdk_watch_os_version(); // __WATCHOS_AVAILABLE(2.0);


// Watch OS only.
// This finds the Watch min OS version that the main executable was built to run on.
// Note: dyld_get_program_min_os_version() returns the iOS equivalent (e.g. 9.0)
//       whereas this returns the raw watchOS version (e.g. 2.0).
// Exists in Watch OS 3.0 and later
extern uint32_t dyld_get_program_min_watch_os_version(); // __WATCHOS_AVAILABLE(3.0);


//
// This finds the min OS version a binary was built to run on.
// Returns zero on error, or if no min OS recorded in binary.
//
// Exists in Mac OS X 10.8 and later 
// Exists in iOS 6.0 and later
extern uint32_t dyld_get_min_os_version(const struct mach_header* mh);


//
// This finds the min OS version the main executable was built to run on.
// Returns zero on error, or if no min OS recorded in binary.
//
// Exists in Mac OS X 10.8 and later 
// Exists in iOS 6.0 and later
extern uint32_t dyld_get_program_min_os_version();




//
// Returns if any OS dylib has overridden its copy in the shared cache
//
// Exists in iPhoneOS 3.1 and later 
// Exists in Mac OS X 10.10 and later
extern bool dyld_shared_cache_some_image_overridden();


	
//
// Returns if the process is setuid or is code signed with entitlements.
//
// Exists in Mac OS X 10.9 and later
extern bool dyld_process_is_restricted();



//
// Returns path used by dyld for standard dyld shared cache file for the current arch.
//
// Exists in Mac OS X 10.11 and later
extern const char* dyld_shared_cache_file_path();



//
// <rdar://problem/13820686> for OpenGL to tell dyld it is ok to deallocate a memory based image when done.
//
// Exists in Mac OS X 10.9 and later
#define NSLINKMODULE_OPTION_CAN_UNLOAD                  0x20


//
// Update all bindings on specified image. 
// Looks for uses of 'replacement' and changes it to 'replacee'.
// NOTE: this is less safe than using static interposing via DYLD_INSERT_LIBRARIES
// because the running program may have already copy the pointer values to other
// locations that dyld does not know about.
//
struct dyld_interpose_tuple {
	const void* replacement;
	const void* replacee;
};
extern void dyld_dynamic_interpose(const struct mach_header* mh, const struct dyld_interpose_tuple array[], size_t count);


struct dyld_shared_cache_dylib_text_info {
	uint64_t		version;		// current version 1
	// following fields all exist in version 1
	uint64_t		loadAddressUnslid;
	uint64_t		textSegmentSize; 
	uuid_t			dylibUuid;
	const char*		path;			// pointer invalid at end of iterations
};
typedef struct dyld_shared_cache_dylib_text_info dyld_shared_cache_dylib_text_info;

//
// Given the UUID of a dyld shared cache file, this function will attempt to locate the cache
// file and if found iterate all images, returning info about each one.  Returns 0 on success.
//
// Exists in Mac OS X 10.11 and later
//           iOS 9.0 and later
extern int dyld_shared_cache_iterate_text(const uuid_t cacheUuid, void (^callback)(const dyld_shared_cache_dylib_text_info* info));


//
// Given the UUID of a dyld shared cache file, and a NULL terminated array of extra directory paths to search,
// this function will scan the standard and extra directories looking for a cache file that matches the UUID
// and if found iterate all images, returning info about each one.  Returns 0 on success.
//
// Exists in Mac OS X 10.12 and later
//           iOS 10.0 and later
extern int dyld_shared_cache_find_iterate_text(const uuid_t cacheUuid, const char* extraSearchDirs[], void (^callback)(const dyld_shared_cache_dylib_text_info* info));


//
// Returns if the specified address range is in a dyld owned memory
// that is mapped read-only and will never be unloaded.
//
// Exists in Mac OS X 10.12 and later
//           iOS 10.0 and later
extern bool _dyld_is_memory_immutable(const void* addr, size_t length);


//
// Finds the UUID (from LC_UUID load command) of given image.
// Returns false if LC_UUID is missing or mach_header is malformed.
//
// Exists in Mac OS X 10.12 and later
// Exists in iOS 10.0 and later
extern bool _dyld_get_image_uuid(const struct mach_header* mh, uuid_t uuid);


//
// Gets the UUID of the dyld shared cache in the current process.
// Returns false if there is no dyld shared cache in use by the processes.
//
// Exists in Mac OS X 10.12 and later
// Exists in iOS 10.0 and later
extern bool _dyld_get_shared_cache_uuid(uuid_t uuid);



//
// When dyld must terminate a process because of a required dependent dylib
// could not be loaded or a symbol is missing, dyld calls abort_with_reason()
// using one of the following error codes.
//
#define DYLD_EXIT_REASON_DYLIB_MISSING          1
#define DYLD_EXIT_REASON_DYLIB_WRONG_ARCH       2
#define DYLD_EXIT_REASON_DYLIB_WRONG_VERSION    3
#define DYLD_EXIT_REASON_SYMBOL_MISSING         4
#define DYLD_EXIT_REASON_CODE_SIGNATURE         5
#define DYLD_EXIT_REASON_FILE_SYSTEM_SANDBOX    6
#define DYLD_EXIT_REASON_MALFORMED_MACHO        7
#define DYLD_EXIT_REASON_OTHER                  9

//
// When it has more information about the termination, dyld will use abort_with_payload().
// The payload is a dyld_abort_payload structure.  The fixed fields are offsets into the
// payload for the corresponding string.  If the offset is zero, that string is not available.
//
struct dyld_abort_payload {
	uint32_t version;                   // first version is 1
	uint32_t flags;                     // 0x00000001 means dyld terminated at launch, backtrace not useful
	uint32_t targetDylibPathOffset;     // offset in payload of path string to dylib that could not be loaded
	uint32_t clientPathOffset;          // offset in payload of path string to image requesting dylib
	uint32_t symbolOffset;              // offset in payload of symbol string that could not be found
	// string data
};
typedef struct dyld_abort_payload dyld_abort_payload;

#if __cplusplus
}
#endif /* __cplusplus */

#endif /* _MACH_O_DYLD_PRIV_H_ */
