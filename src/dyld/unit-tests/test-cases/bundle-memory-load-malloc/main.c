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
#include <mach-o/dyld.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <sys/mman.h> 
#include <unistd.h>
#include <fcntl.h>
#include <malloc/malloc.h>
#include <stdlib.h>
#include <Availability.h>


#include "test.h" // PASS(), FAIL()

typedef bool (*CheckFunc)();

int main()
{
// NSCreateObjectFileImageFromMemory is only available on Mac OS X - not iPhone OS
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	int fd = open("test.bundle", O_RDONLY, 0);
	if ( fd == -1 ) {
		FAIL("open() failed");
		return 1;
	}

	struct stat stat_buf;
	if ( fstat(fd, &stat_buf) == -1) {
		FAIL("fstat() failed");
		return 1;
	}

	void* loadAddress = malloc((stat_buf.st_size+4095) & (-4096));
	if ( loadAddress == NULL ) {
		FAIL("malloc failed");
		return 1;
	}
	
	if ( pread(fd, loadAddress, stat_buf.st_size, 0) != stat_buf.st_size ) {
		FAIL("pread() failed");
		return 1;
	}

	//void* loadAddress2 = mmap(NULL, stat_buf.st_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
	close(fd);

	NSObjectFileImage ofi;
	if ( NSCreateObjectFileImageFromMemory(loadAddress, stat_buf.st_size, &ofi) != NSObjectFileImageSuccess ) {
		FAIL("NSCreateObjectFileImageFromMemory failed");
		return 1;
	}
	
	NSModule mod = NSLinkModule(ofi, "he_he", NSLINKMODULE_OPTION_NONE);
	if ( mod == NULL ) {
		FAIL("NSLinkModule failed");
		return 1;
	}
	
	NSSymbol sym = NSLookupSymbolInModule(mod, "_checkdata");
	if ( sym == NULL ) {
		FAIL("NSLookupSymbolInModule failed");
		return 1;
	}

	CheckFunc func = NSAddressOfSymbol(sym);
	if ( !func() ) {
		FAIL("NSAddressOfSymbol failed");
		return 1;
	}

	if ( !NSUnLinkModule(mod, NSUNLINKMODULE_OPTION_NONE) ) {
		FAIL("NSUnLinkModule failed");
		return 1;
	}

	if ( !NSDestroyObjectFileImage(ofi) ) {
		FAIL("NSDestroyObjectFileImage failed");
		return 1;
	}
	//fprintf(stderr, "loadAddress=%p\n", loadAddress);
	//fprintf(stderr, "malloc_size(loadAddress) => 0x%08X\n", malloc_size(loadAddress));
	//fprintf(stderr, "loadAddress2=%p\n", loadAddress2);
	//fprintf(stderr, "malloc_size(loadAddress2) => 0x%08X\n", malloc_size(loadAddress2));
	
	
	//free(loadAddress);
	//fprintf(stderr, "malloc_size(loadAddress) => 0x%08X\n", malloc_size(loadAddress));
	if ( malloc_size(loadAddress) != 0 ) {
		FAIL("malloc_size(loadAddress) => 0x%08X", malloc_size(loadAddress));
		FAIL("malloc still thinks it owns this block");
		return 1;
	}
	
	// Should check that loadAddress is unmmaped now (by call to NSDestroyObjectFileImage)
#endif

	PASS("bundle-memory-load-malloc");
	return 0;
}