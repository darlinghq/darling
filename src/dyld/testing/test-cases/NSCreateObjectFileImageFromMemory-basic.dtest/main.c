// BUILD_ONLY: MacOSX

// BUILD:  $CC main.c  -o $BUILD_DIR/NSCreateObjectFileImageFromMemory-basic.exe -Wno-deprecated-declarations
// BUILD:  $CC foo.c   -o $BUILD_DIR/foo.bundle -bundle

// RUN:  ./NSCreateObjectFileImageFromMemory-basic.exe $RUN_DIR/foo.bundle



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <sys/mman.h> 
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>


static void checkBundle(const char* path, bool unlinkBeforeDestroy)
{
	int fd = open(path, O_RDONLY, 0);
	if ( fd == -1 ) {
		printf("[FAIL] open(%s) failed", path);
		exit(0);
	}

	struct stat stat_buf;
	if ( fstat(fd, &stat_buf) == -1) {
		printf("[FAIL] fstat() failed\n");
		exit(0);
	}

	void* loadAddress = mmap(NULL, stat_buf.st_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
	if ( loadAddress == ((void*)(-1)) ) {
		printf("[FAIL] mmap() failed\n");
		exit(0);
	}

	close(fd);

	NSObjectFileImage ofi;
	if ( NSCreateObjectFileImageFromMemory(loadAddress, stat_buf.st_size, &ofi) != NSObjectFileImageSuccess ) {
		printf("[FAIL] NSCreateObjectFileImageFromMemory failed\n");
		exit(0);
	}

	NSModule mod = NSLinkModule(ofi, path, NSLINKMODULE_OPTION_NONE);
	if ( mod == NULL ) {
		printf("[FAIL] NSLinkModule failed\n");
		exit(0);
	}
	
   if ( !unlinkBeforeDestroy ) {
        // API lets you destroy ofi and NSModule lives on
        if ( !NSDestroyObjectFileImage(ofi) ) {
            printf("[FAIL] NSDestroyObjectFileImage failed\n");
            exit(0);
        }
    }

	NSSymbol sym = NSLookupSymbolInModule(mod, "_fooInBundle");
	if ( sym == NULL ) {
		printf("[FAIL] NSLookupSymbolInModule failed\n");
		exit(0);
	}

	void* func = NSAddressOfSymbol(sym);
	if ( func == NULL ) {
		printf("[FAIL] NSAddressOfSymbol failed\n");
		exit(0);
	}

    Dl_info info;
    if ( dladdr(func, &info) == 0 ) {
        printf("[FAIL] dladdr(&p, xx) failed\n");
        exit(0);
    }
    //printf("_fooInBundle found in %s\n", info.dli_fname);

    if ( !NSUnLinkModule(mod, NSUNLINKMODULE_OPTION_NONE) ) {
            printf("[FAIL] NSUnLinkModule failed\n");
            exit(0);
    }

    if ( dladdr(func, &info) != 0 ) {
        printf("[FAIL] dladdr(&p, xx) found but should not have\n");
        exit(0);
    }

    if ( unlinkBeforeDestroy ) {
        if ( !NSDestroyObjectFileImage(ofi) ) {
            printf("[FAIL] NSDestroyObjectFileImage failed\n");
            exit(0);
        }
    }
}


int main(int argc, const char* argv[])
{
    printf("[BEGIN] NSCreateObjectFileImageFromMemory-basic\n");

    checkBundle(argv[1], true);
    checkBundle(argv[1], false);

    // Now go again enough times to flush out any limits in our dlopen encodings.
    for (unsigned i = 0; i != 255; ++i)
      checkBundle(argv[1], false);

    printf("[PASS] NSCreateObjectFileImageFromMemory-basic\n");
    return 0;
}

