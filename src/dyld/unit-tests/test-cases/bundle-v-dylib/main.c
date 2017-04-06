/*
 * Copyright (c) 2005-2009 Apple Inc. All rights reserved.
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
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <mach-o/dyld.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <sys/mman.h> 
#include <unistd.h>
#include <fcntl.h>
#include <Availability.h>

#include "test.h" // PASS(), FAIL()

extern void bar();


// NSCreateObjectFileImageFromMemory is only available on Mac OS X - not iPhone OS
#if __MAC_OS_X_VERSION_MIN_REQUIRED

void loadAsBundleFromMemory(const char* path)
{
	int fd = open(path, O_RDONLY, 0);
	if ( fd == -1 ) {
		FAIL("bundle-v-dylib: open() failed");
		exit(0);
	}

	struct stat stat_buf;
	if ( fstat(fd, &stat_buf) == -1) {
		FAIL("bundle-v-dylib: fstat() failed");
		exit(0);
	}

	void* loadAddress = mmap(NULL, stat_buf.st_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
	if ( loadAddress == ((void*)(-1)) ) {
		FAIL("bundle-v-dylib: mmap() failed");
		exit(0);
	}

	close(fd);

	NSObjectFileImage ofi;
	if ( NSCreateObjectFileImageFromMemory(loadAddress, stat_buf.st_size, &ofi) == NSObjectFileImageSuccess ) {
		FAIL("bundle-v-dylib: NSCreateObjectFileImageFromMemory() incorrectly allowed %s to be loaded", path);
		exit(0);
	}
}

void loadAsBundle(const char* path)
{
	NSObjectFileImage ofi;
	if ( NSCreateObjectFileImageFromFile(path, &ofi) == NSObjectFileImageSuccess ) {
		FAIL("bundle-v-dylib: NSCreateObjectFileImageFromFile() incorrectly allowed %s to be loaded", path);
		exit(0);
	}
}

void loadAsDylib(const char* path)
{
	if ( NSAddImage(path, NSADDIMAGE_OPTION_RETURN_ON_ERROR) != NULL ) {
		FAIL("bundle-v-dylib: NSAddImage() incorrectly allowed %s to be loaded", path);
		exit(0);
	}
}

#endif

int main()
{
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	int dummy; 
	
	// verify that NSAddImage fails to load MH_BUNDLE
	loadAsDylib("foo.bundle");

	// verify that NSCreateObjectFileImageFromFile fails to load MH_DYLIB
	loadAsBundle("foo.dylib");

	// verify that NSCreateObjectFileImageFromFile fails to load MH_DYLIB already linked against main
	loadAsBundle("bar.dylib");
	
	// verify that bar.dylib was not unloaded when above failed
	bar();
	
	// try loading a dylib from memory using bundle API's
	loadAsBundleFromMemory("foo2.dylib");
	
	// verify that dyld data structures are not wanked by scanning all images
	_dyld_get_image_header_containing_address(&dummy);
#endif
	PASS("bundle-v-dylib");
	return 0;
}