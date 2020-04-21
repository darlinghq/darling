
// BUILD:  $CC main.c  -o $BUILD_DIR/dlopen-long-error-message.exe

// RUN:  ./dlopen-long-error-message.exe

#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>



int main()
{
    printf("[BEGIN] dlopen-long-error-message\n");

    for (int i=0; i < 10; ++i) {
        void* handle = dlopen("/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/bogus/path/libbogus.dylib", RTLD_FIRST);
        if ( handle != NULL ) {
            printf("[FAIL] dlopen-long-error-message should have failed on non-existent file\n");
            return 0;
        }
        free(strdup("hello there"));
    }

    printf("[PASS] dlopen-long-error-message\n");

	return 0;
}

