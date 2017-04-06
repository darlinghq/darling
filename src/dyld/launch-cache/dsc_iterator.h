/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*- 
 *
 * Copyright (c) 2009-2012 Apple Inc. All rights reserved.
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

#include <stdint.h>
#include <uuid/uuid.h>

struct dyld_shared_cache_dylib_info {
	uint32_t		version;		// current version 2
	// following fields all exist in version 1
	uint32_t		isAlias;		// this is alternate path (symlink)
	const void*		machHeader;		// of dylib in mapped cached file
	const char*		path;			// of dylib
	const uuid_t*	uuid;			// of dylib, or NULL is missing
	// following fields all exist in version 2
	uint64_t		inode;			// of dylib file or path hash
	uint64_t		modTime;		// of dylib file
};
typedef struct dyld_shared_cache_dylib_info dyld_shared_cache_dylib_info;

struct dyld_shared_cache_segment_info {
	uint64_t		version;		// initial version 1
	const char*		name;			// of segment
	uint64_t		fileOffset;		// of segment in cache file
	uint64_t		fileSize;		// of segment
	uint64_t		address;		// of segment when cache mapped with ASLR (sliding) off
	// above fields all exist in version 1
};
typedef struct dyld_shared_cache_segment_info dyld_shared_cache_segment_info;


#ifdef __cplusplus
extern "C" {
#endif 


// Given a pointer and size of an in-memory copy of a dyld shared cache file,
// this routine will call the callback block once for each segment in each dylib 
// in the shared cache file.  
// Returns -1 if there was an error, otherwise 0.
extern int dyld_shared_cache_iterate(const void* shared_cache_file, uint32_t shared_cache_size, 
									void (^callback)(const dyld_shared_cache_dylib_info* dylibInfo, const dyld_shared_cache_segment_info* segInfo));



//
// The following iterator functions are deprecated:
//
typedef void (^dyld_shared_cache_iterator_t)(const char* dylibName, const char* segName, uint64_t offset, uint64_t size, uint64_t mappedddress);
typedef void (^dyld_shared_cache_iterator_slide_t)(const char* dylibName, const char* segName, uint64_t offset, uint64_t size, uint64_t mappedddress, uint64_t slide);
typedef void (*dyld_shared_cache_iterator_nb_t)(const char* dylibName, const char* segName, uint64_t offset, uint64_t sizem, uint64_t mappedddress, void* userData);
typedef void (*dyld_shared_cache_iterator_slide_nb_t)(const char* dylibName, const char* segName, uint64_t offset, uint64_t sizem, uint64_t mappedddress, uint64_t slide, void* userData);

extern int dyld_shared_cache_iterate_segments(const void* shared_cache_file, dyld_shared_cache_iterator_t callback) __attribute__((deprecated));
extern int dyld_shared_cache_iterate_segments_with_slide(const void* shared_cache_file, dyld_shared_cache_iterator_slide_t callback) __attribute__((deprecated));
extern int dyld_shared_cache_iterate_segments_nb(const void* shared_cache_file, dyld_shared_cache_iterator_nb_t callback, void* userData) __attribute__((deprecated));
extern int dyld_shared_cache_iterate_segments_with_slide_nb(const void* shared_cache_file, dyld_shared_cache_iterator_slide_nb_t callback, void* userData) __attribute__((deprecated));


#ifdef __cplusplus
}
#endif 
