
// BUILD:  $CC main.c           -o $BUILD_DIR/foo.exe
// BUILD:  $CC main.c           -o $BUILD_DIR/dyld_need_closure.exe

// RUN:    ./dyld_need_closure.exe

#include <stdio.h>
#include <stdlib.h>
#include <mach-o/dyld_priv.h>


int main()
{
    printf("[BEGIN] dyld_need_closure\n");

    // We only support trying to save to containerised paths, so anything not
    // of that form should fail
    if ( !dyld_need_closure("./foo.exe", "/tmp/Containers/Data/") ) {
        printf("[FAIL] dyld_closure: Should have needed a closure for containerised path\n");
        return 0;
    }

    if ( dyld_need_closure("./foo.exe", "/tmp/Containers/Data2/") ) {
        printf("[FAIL] dyld_closure: Should have rejected a closure for non-containerised path\n");
        return 0;
    }

    printf("[PASS] dyld_need_closure\n");

    return 0;
}
