/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
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
#include <stdlib.h> 
#include <stdbool.h>
#include <mach-o/dyld.h>
#include <mach-o/dyld_images.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <sys/mman.h> 
#include <unistd.h>
#include <fcntl.h>
#include <Availability.h>

#include "test.h" // PASS(), FAIL()


struct dyld_all_image_infos* getImageInfosFromKernel()
{
	task_dyld_info_data_t task_dyld_info;
	mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
    
    if ( task_info(mach_task_self(), TASK_DYLD_INFO, (task_info_t)&task_dyld_info, &count) ) {
		FAIL("all_image_infos: task_info() failed");
		exit(0);
	}
	return (struct dyld_all_image_infos*)(uintptr_t)task_dyld_info.all_image_info_addr;
}


int main()
{
// NSObjectFileImage APIs are only available on Mac OS X - not iPhone OS
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	int fd = open("test.bundle", O_RDONLY, 0);
	if ( fd == -1 ) {
		FAIL("open() failed");
		return 1;
	}

	struct stat stat_buf;
	if ( fstat(fd, &stat_buf) == -1) {
		FAIL("fstat() failed");
		return 0;
	}

	void* loadAddress = mmap(NULL, stat_buf.st_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
	if ( loadAddress == ((void*)(-1)) ) {
		FAIL("mmap() failed");
		return 0;
	}

	close(fd);

	NSObjectFileImage ofi;
	if ( NSCreateObjectFileImageFromMemory(loadAddress, stat_buf.st_size, &ofi) != NSObjectFileImageSuccess ) {
		FAIL("NSCreateObjectFileImageFromMemory failed");
		return 0;
	}
	
	NSModule mod = NSLinkModule(ofi, "he_he", NSLINKMODULE_OPTION_NONE);
	if ( mod == NULL ) {
		FAIL("NSLinkModule failed");
		return 0;
	}
	
	// look for he_he string in list of images loaded
	struct dyld_all_image_infos* infos = getImageInfosFromKernel();

	if ( infos->infoArrayCount < 2 ) {
		FAIL("bundle-memory-load-all-images: dyld_all_image_infos.infoArrayCount is  < 2");
		return 0;
	}

	bool found = false;
	for( int i=0; i < infos->infoArrayCount; ++i) {
		//fprintf(stderr, "infos->infoArray[%d].imageLoadAddress=%p %s\n", i, infos->infoArray[i].imageLoadAddress, infos->infoArray[i].imageFilePath);
		if ( infos->infoArray[i].imageFilePath == NULL ) {
			FAIL("bundle-memory-load-all-images: NULL image path found");
			exit(0);
		}
		if ( strcmp(infos->infoArray[i].imageFilePath, "he_he") == 0 )
			found = true;
	}
	
	if ( !found ) {
		FAIL("bundle-memory-load-all-images: loaded memory bundle 'he_he' nout found");
		return 0;
	}

	if ( !NSUnLinkModule(mod, NSUNLINKMODULE_OPTION_NONE) ) {
		FAIL("NSUnLinkModule failed");
		return 0;
	}

	if ( !NSDestroyObjectFileImage(ofi) ) {
		FAIL("NSDestroyObjectFileImage failed");
		return 0;
	}
	
	// Should check that loadAddress is unmmaped now (by call to NSDestroyObjectFileImage)
#endif

	PASS("bundle-memory-load-all-images");
	return 0;
}