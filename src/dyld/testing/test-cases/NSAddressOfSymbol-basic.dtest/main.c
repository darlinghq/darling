// BUILD_ONLY: MacOSX

// BUILD:  $CC main.c            -o $BUILD_DIR/NSAddressOfSymbol-basic.exe -Wno-deprecated-declarations

// RUN:  ./NSAddressOfSymbol-basic.exe



#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>

extern struct mach_header __dso_handle;

int main(int argc, const char* argv[])
{
    printf("[BEGIN] NSAddressOfSymbol-basic\n");

    NSSymbol sym = NSLookupSymbolInImage(&__dso_handle, "_main", NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR);
    if ( sym == NULL ) {
        printf("[FAIL] NSAddressOfSymbol-basic can't find main\n");
        return 0;
    }
    void* mainAddr = NSAddressOfSymbol(sym);
    if ( mainAddr != &main ) {
        printf("[FAIL] NSAddressOfSymbol-basic address returned %p is not &main=%p\n", mainAddr, &main);
        return 0;
    }

    // verify NULL works
    if ( NSAddressOfSymbol(NULL) != NULL ) {
        printf("[FAIL] NSAddressOfSymbol-basic NULL not handle\n");
        return 0;
    }

    printf("[PASS] NSAddressOfSymbol-basic\n");
	return 0;
}

