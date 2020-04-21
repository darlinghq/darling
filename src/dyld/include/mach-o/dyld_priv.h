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

#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include <Availability.h>
#include <TargetConditionals.h>
#include <mach-o/dyld.h>
#include <uuid/uuid.h>

#if __cplusplus
extern "C" {
#endif /* __cplusplus */



//
// private interface between libSystem.dylib and dyld
//
extern void _dyld_atfork_prepare(void);
extern void _dyld_atfork_parent(void);
extern void _dyld_fork_child(void);



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
// get slide for a given loaded mach_header  
// Mac OS X 10.6 and later
//
extern intptr_t _dyld_get_image_slide(const struct mach_header* mh);



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
#if !__USING_SJLJ_EXCEPTIONS__
extern bool _dyld_find_unwind_sections(void* addr, struct dyld_unwind_sections* info);
#endif


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

typedef uint32_t dyld_platform_t;

typedef struct {
    dyld_platform_t platform;
    uint32_t        version;
} dyld_build_version_t;

// Returns the active platform of the process
extern dyld_platform_t dyld_get_active_platform(void) __API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));

// Base platforms are platforms that have version numbers (macOS, iOS, watchos, tvOS, bridgeOS)
// All other platforms are mapped to a base platform for version checks
extern dyld_platform_t dyld_get_base_platform(dyld_platform_t platform) __API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));

// SPI to ask if a platform is a simulation platform
extern bool dyld_is_simulator_platform(dyld_platform_t platform) __API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));

// Takes a version and returns if the image was built againt that SDK or newer
// In the case of multi_plaform mach-o's it tests against the active platform
extern bool dyld_sdk_at_least(const struct mach_header* mh, dyld_build_version_t version) __API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));

// Takes a version and returns if the image was built with that minos version or newer
// In the case of multi_plaform mach-o's it tests against the active platform
extern bool dyld_minos_at_least(const struct mach_header* mh, dyld_build_version_t version) __API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));

// Convenience versions of the previous two functions that run against the the main executable
extern bool dyld_program_sdk_at_least(dyld_build_version_t version) __API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));
extern bool dyld_program_minos_at_least(dyld_build_version_t version) __API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));

// Function that walks through the load commands and calls the internal block for every version found
// Intended as a fallback for very complex (and rare) version checks, or for tools that need to
// print our everything for diagnostic reasons
extern void dyld_get_image_versions(const struct mach_header* mh, void (^callback)(dyld_platform_t platform, uint32_t sdk_version, uint32_t min_version)) __API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));

// Convienence constants for dyld version SPIs.

//@VERSION_SET_DEFS@

//@MACOS_PLATFORM_VERSION_DEFS@

//@IOS_PLATFORM_VERSION_DEFS@

//@WATCHOS_PLATFORM_VERSION_DEFS@

//@TVOS_PLATFORM_VERSION_DEFS@

//@BRIDGEOS_PLATFORM_VERSION_DEFS@

// Convienence constants for return values from dyld_get_sdk_version() and friends.

//@MAC_VERSION_DEFS@

//@IOS_VERSION_DEFS@

//@WATCHOS_VERSION_DEFS@

//@TVOS_VERSION_DEFS@

//@BRIDGEOS_VERSION_DEFS@

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
// Note on watchOS, this returns the equivalent iOS SDK version number
// (i.e an app built against watchOS 2.0 SDK returne 9.0).  To see the
// platform specific sdk version use dyld_get_program_sdk_watch_os_version().
//
// Exists in Mac OS X 10.8 and later 
// Exists in iOS 6.0 and later
extern uint32_t dyld_get_program_sdk_version(void);

#if TARGET_OS_WATCH
// watchOS only.
// This finds the Watch OS SDK version that the main executable was built against.
// Exists in Watch OS 2.0 and later
extern uint32_t dyld_get_program_sdk_watch_os_version(void) __API_AVAILABLE(watchos(2.0));


// watchOS only.
// This finds the Watch min OS version that the main executable was built to run on.
// Note: dyld_get_program_min_os_version() returns the iOS equivalent (e.g. 9.0)
//       whereas this returns the raw watchOS version (e.g. 2.0).
// Exists in Watch OS 3.0 and later
extern uint32_t dyld_get_program_min_watch_os_version(void) __API_AVAILABLE(watchos(3.0));
#endif

#if TARGET_OS_BRIDGE
// bridgeOS only.
// This finds the bridgeOS SDK version that the main executable was built against.
// Exists in bridgeOSOS 2.0 and later
extern uint32_t dyld_get_program_sdk_bridge_os_version(void) __API_AVAILABLE(bridgeos(2.0));

// bridgeOS only.
// This finds the Watch min OS version that the main executable was built to run on.
// Note: dyld_get_program_min_os_version() returns the iOS equivalent (e.g. 9.0)
//       whereas this returns the raw bridgeOS version (e.g. 2.0).
// Exists in bridgeOS 2.0 and later
extern uint32_t dyld_get_program_min_bridge_os_version(void) __API_AVAILABLE(bridgeos(2.0));
#endif

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
extern uint32_t dyld_get_program_min_os_version(void);




//
// Returns if any OS dylib has overridden its copy in the shared cache
//
// Exists in iPhoneOS 3.1 and later 
// Exists in Mac OS X 10.10 and later
extern bool dyld_shared_cache_some_image_overridden(void);


	
//
// Returns if the process is setuid or is code signed with entitlements.
//
// Exists in Mac OS X 10.9 and later
extern bool dyld_process_is_restricted(void);



//
// Returns path used by dyld for standard dyld shared cache file for the current arch.
//
// Exists in Mac OS X 10.11 and later
extern const char* dyld_shared_cache_file_path(void);



//
// Returns if there are any inserted (via DYLD_INSERT_LIBRARIES) or interposing libraries.
//
// Exists in Mac OS X 10.15 and later
extern bool dyld_has_inserted_or_interposing_libraries(void);


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
	// following fields all exist in version 2
	uint64_t        textSegmentOffset;  // offset from start of cache
};
typedef struct dyld_shared_cache_dylib_text_info dyld_shared_cache_dylib_text_info;


#ifdef __BLOCKS__
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
#endif /* __BLOCKS */


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
// Returns the start address of the dyld cache in the process and sets length to the size of the cache.
// Returns NULL if the process is not using a dyld shared cache
//
// Exists in Mac OS X 10.13 and later
// Exists in iOS 11.0 and later
extern const void* _dyld_get_shared_cache_range(size_t* length);


//
// Returns if the currently active dyld shared cache is optimized.
// Note: macOS does not use optimized caches and will always return false.
//
// Exists in Mac OS X 10.15 and later
// Exists in iOS 13.0 and later
extern bool _dyld_shared_cache_optimized(void);


//
// Returns if the currently active dyld shared cache was built locally.
//
// Exists in Mac OS X 10.15 and later
// Exists in iOS 13.0 and later
extern bool _dyld_shared_cache_is_locally_built(void);

//
// Returns if the given app needs a closure built.
//
// Exists in Mac OS X 10.15 and later
// Exists in iOS 13.0 and later
extern bool dyld_need_closure(const char* execPath, const char* tempDir);


struct dyld_image_uuid_offset {
    uuid_t                      uuid;
	uint64_t                    offsetInImage;
    const struct mach_header*   image;
};

//
// Given an array of addresses, returns info about each address.
// Common usage is the array or addresses was produced by a stack backtrace.
// For each address, returns the where that image was loaded, the offset
// of the address in the image, and the image's uuid.  If a specified
// address is unknown to dyld, all fields will be returned a zeros.
//
// Exists in macOS 10.14 and later
// Exists in iOS 12.0 and later
extern void _dyld_images_for_addresses(unsigned count, const void* addresses[], struct dyld_image_uuid_offset infos[]);


//
// Lets you register a callback which is called each time an image is loaded and provides the mach_header*, path, and
// whether the image may be unloaded later.  During the call to _dyld_register_for_image_loads(), the callback is called
// once for each image currently loaded.
//
// Exists in macOS 10.14 and later
// Exists in iOS 12.0 and later
extern void _dyld_register_for_image_loads(void (*func)(const struct mach_header* mh, const char* path, bool unloadable));




//
// Lets you register a callback which is called for bulk notifications of images loaded. During the call to
// _dyld_register_for_bulk_image_loads(), the callback is called once with all images currently loaded.
// Then later during dlopen() the callback is called once with all newly images.
//
// Exists in macOS 10.15 and later
// Exists in iOS 13.0 and later
extern void _dyld_register_for_bulk_image_loads(void (*func)(unsigned imageCount, const struct mach_header* mhs[], const char* paths[]));


//
// DriverKit main executables do not have an LC_MAIN.  Instead DriverKit.framework's initializer calls
// _dyld_register_driverkit_main() with a function pointer that dyld should call into instead
// of using LC_MAIN.
//
extern void _dyld_register_driverkit_main(void (*mainFunc)(void));



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


// These global variables are implemented in libdyld.dylib
// Old programs that used crt1.o also defined these globals.
// The ones in dyld are not used when an old program is run.
extern int          NXArgc;
extern const char** NXArgv;
extern       char** environ;       // POSIX says this not const, because it pre-dates const
extern const char*  __progname;


// called by libSystem_initializer only
extern void _dyld_initializer(void);

// never called from source code. Used by static linker to implement lazy binding
extern void dyld_stub_binder(void) __asm__("dyld_stub_binder");

// never call from source code.  Used by closure builder to bind missing lazy symbols to
extern void _dyld_missing_symbol_abort(void);

// Called only by objc to see if dyld has uniqued this selector.
// Returns the value if dyld has uniqued it, or nullptr if it has not.
// Note, this function must be called after _dyld_objc_notify_register.
//
// Exists in Mac OS X 10.15 and later
// Exists in iOS 13.0 and later
extern const char* _dyld_get_objc_selector(const char* selName);


// Called only by objc to see if dyld has pre-optimized classes with this name.
// The callback will be called once for each class with the given name where
// isLoaded is true if that class is in a binary which has been previously passed
// to the objc load notifier.
// Note you can set stop to true to stop iterating.
// Also note, this function must be called after _dyld_objc_notify_register.
//
// Exists in Mac OS X 10.15 and later
// Exists in iOS 13.0 and later
extern void _dyld_for_each_objc_class(const char* className,
                                      void (^callback)(void* classPtr, bool isLoaded, bool* stop));


// Called only by objc to see if dyld has pre-optimized protocols with this name.
// The callback will be called once for each protocol with the given name where
// isLoaded is true if that protocol is in a binary which has been previously passed
// to the objc load notifier.
// Note you can set stop to true to stop iterating.
// Also note, this function must be called after _dyld_objc_notify_register.
//
// Exists in Mac OS X 10.15 and later
// Exists in iOS 13.0 and later
extern void _dyld_for_each_objc_protocol(const char* protocolName,
                                         void (^callback)(void* protocolPtr, bool isLoaded, bool* stop));


// called by exit() before it calls cxa_finalize() so that thread_local
// objects are destroyed before global objects.
extern void _tlv_exit(void);


// temp exports to keep tapi happy, until ASan stops using dyldVersionNumber
extern double      dyldVersionNumber;
extern const char* dyldVersionString;

#if __cplusplus
}
#endif /* __cplusplus */

#endif /* _MACH_O_DYLD_PRIV_H_ */
