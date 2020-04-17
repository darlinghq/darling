// BUILD:  $CC main.c -o $BUILD_DIR/dlsym-in-interposed-malloc.exe
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/dlsym-in-interposed-malloc.exe
// BUILD:  $CC interposer.c -dynamiclib -o $BUILD_DIR/libmyalloc.dylib -install_name libmyalloc.dylib

// RUN:    DYLD_INSERT_LIBRARIES=libmyalloc.dylib   ./dlsym-in-interposed-malloc.exe


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    printf("[BEGIN] dlsym-in-interposed-malloc\n");

    // malloc should have been called when dyld3's libdyld was initialized, but
    // call it one more time anyway just to make sure its working
    (void)malloc(1);
    

    //printf("%p %p %p %p\n", p1, p2, p3, p4);
    printf("[PASS] dlsym-in-interposed-malloc\n");
	return 0;
}
