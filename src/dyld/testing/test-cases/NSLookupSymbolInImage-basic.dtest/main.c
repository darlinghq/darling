// BUILD_ONLY: MacOSX

// BUILD:  $CC main.c            -o $BUILD_DIR/NSLookupSymbolInImage-basic.exe -Wno-deprecated-declarations

// RUN:  ./NSLookupSymbolInImage-basic.exe


#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>

extern struct mach_header __dso_handle;

int main(int argc, const char* argv[])
{
    printf("[BEGIN] NSLookupSymbolInImage-basic\n");

    // verify known symbol works
    NSSymbol sym = NSLookupSymbolInImage(&__dso_handle, "_main", NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR);
    if ( sym == NULL ) {
         printf("[FAIL] NSLookupSymbolInImage-basic _main\n");
         return 0;
    }

    // verify mode where NSLookupSymbolInImage() returns NULL if symbol not found
    sym = NSLookupSymbolInImage(&__dso_handle, "_42hhg", NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR);
    if ( sym != NULL ) {
         printf("[FAIL] NSLookupSymbolInImage-basic _42hhg\n");
         return 0;
    }

    // Note: NSLookupSymbolInImage is documented to abort if symbol not found and NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR not used,
    // but dyld 2 just returned NULL, so no need to test that.

    printf("[PASS] NSLookupSymbolInImage-basic\n");
	return 0;
}

