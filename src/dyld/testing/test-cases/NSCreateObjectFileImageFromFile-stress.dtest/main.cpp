// BUILD_ONLY: MacOSX

// BUILD:  $CXX main.cpp  -o $BUILD_DIR/NSCreateObjectFileImageFromFile-stress.exe -Wno-deprecated-declarations
// BUILD:  $CC  foo.c     -o $BUILD_DIR/foo.bundle -bundle

// RUN:  ./NSCreateObjectFileImageFromFile-stress.exe $RUN_DIR/foo.bundle


#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <vector>

int main(int argc, const char* argv[])
{
    printf("[BEGIN] NSCreateObjectFileImageFromFile-basic\n");

    const char* path = argv[1];

    std::vector<NSObjectFileImage> ofis;
    for (unsigned i = 0; i != 32; ++i) {
		NSObjectFileImage ofi;
		if ( NSCreateObjectFileImageFromFile(path, &ofi) != NSObjectFileImageSuccess ) {
			printf("[FAIL] NSCreateObjectFileImageFromFile failed\n");
			return 0;
		}
		ofis.push_back(ofi);
	}
	
	for (unsigned i = 0; i != 32; ++i) {
		NSObjectFileImage ofi = ofis[i];
		NSModule mod = NSLinkModule(ofi, path, NSLINKMODULE_OPTION_NONE);
		if ( mod == NULL ) {
			printf("[FAIL] NSLinkModule failed\n");
			return 0;
		}
		
		NSSymbol sym = NSLookupSymbolInModule(mod, "_fooInBundle");
		if ( sym == NULL ) {
			printf("[FAIL] NSLookupSymbolInModule failed\n");
			return 0;
		}

		void* func = NSAddressOfSymbol(sym);
		if ( func == NULL ) {
			printf("[FAIL] NSAddressOfSymbol failed\n");
			return 0;
		}

	    Dl_info info;
	    if ( dladdr(func, &info) == 0 ) {
	        printf("[FAIL] dladdr(&p, xx) failed");
			return 0;
	    }
	    //printf("_fooInBundle found in %s\n", info.dli_fname);

		if ( !NSUnLinkModule(mod, NSUNLINKMODULE_OPTION_NONE) ) {
			printf("[FAIL] NSUnLinkModule failed\n");
			return 0;
		}

	    if ( dladdr(func, &info) != 0 ) {
	        printf("[FAIL] dladdr(&p, xx) found but should not have\n");
			return 0;
	    }
	}

	for (unsigned i = 0; i != 32; ++i) {
		NSObjectFileImage ofi = ofis[i];
		if ( !NSDestroyObjectFileImage(ofi) ) {
			printf("[FAIL] NSDestroyObjectFileImage failed\n");
			return 0;
		}
	}

    printf("[PASS] NSCreateObjectFileImageFromFile-basic\n");
	return 0;
}

