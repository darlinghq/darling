
// BUILD:  $CC main-no-syms.c            -o $BUILD_DIR/dladdr-stripped.exe
// BUILD:  strip $BUILD_DIR/dladdr-stripped.exe

// RUN:  ./dladdr-stripped.exe


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld_priv.h>



///
/// verify dladdr() returns NULL for a symbol name in a fully stripped 
/// main executable (and not _mh_execute_header+nnn).
///

int main()
{
    printf("[BEGIN] dladdr-stripped\n");

    Dl_info info;
    if ( dladdr(&main, &info) == 0 ) {
        printf("[FAIL] dladdr(&main, xx) failed\n");
        return 0;
    }

    if ( info.dli_sname != NULL ){
        printf("[FAIL] dladdr() returned: \"%s\" instead of NULL\n", info.dli_sname);
        return 0;
    }

    printf("[PASS] dladdr-stripped\n");
    return 0;
}
