
// BUILD:  $CC init-b.c -dynamiclib -DRUN_DIR="$RUN_DIR" -install_name $RUN_DIR/libInitB.dylib -o $BUILD_DIR/libInitB.dylib
// BUILD:  $CC init-a.c -dynamiclib                      -install_name $RUN_DIR/libInitA.dylib $BUILD_DIR/libInitB.dylib -o $BUILD_DIR/libInitA.dylib
// BUILD:  $CC init-main.c $BUILD_DIR/libInitA.dylib -o $BUILD_DIR/dlopen-RTLD_NOLOAD-in-initializer.exe

// RUN:  ./dlopen-RTLD_NOLOAD-in-initializer.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dlfcn.h>


extern bool initsInWrongOrder;
extern bool allInitsDone();

int main()
{
    printf("[BEGIN] dlopen-RTLD_NOLOAD-in-initializer\n");

    ///
    /// This tests that using RTLD_NOLOAD in an initializer does not trigger out of order initializers
    ///
    if ( initsInWrongOrder )
        printf("[FAIL] dlopen-RTLD_NOLOAD-in-initializer: wrong init order\n");
    else if ( !allInitsDone() )
        printf("[FAIL] dlopen-RTLD_NOLOAD-in-initializer: all initializers not run\n");
    else
        printf("[PASS] dlopen-RTLD_NOLOAD-in-initializer\n");

    return 0;
}
