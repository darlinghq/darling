
// BUILD:  $CC main.c            -o $BUILD_DIR/_dyld_shared_cache_is_locally_built.exe

// RUN:  ./_dyld_shared_cache_is_locally_built.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <mach-o/dyld_priv.h>
#include <dlfcn.h>


int main()
{
    printf("[BEGIN] _dyld_shared_cache_is_locally_built\n");

    // We can't reliably test the result of this function, but it shouldn't crash
    _dyld_shared_cache_is_locally_built();

    printf("[PASS] _dyld_shared_cache_is_locally_built\n");

    return 0;
}

