
// BUILD:  $CC main.c            -o $BUILD_DIR/shared_cache_optimized.exe

// RUN:  ./shared_cache_optimized.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <mach-o/dyld_priv.h>
#include <dlfcn.h>


int main()
{
    printf("[BEGIN] shared_cache_optimized\n");

    // tests run on internal installs which use un-optimzed dyld cache
    if ( _dyld_shared_cache_optimized() )
        printf("[FAIL] shared_cache_optimized unexpectedly returned true\n");
    else
        printf("[PASS] shared_cache_optimized\n");

    return 0;
}

