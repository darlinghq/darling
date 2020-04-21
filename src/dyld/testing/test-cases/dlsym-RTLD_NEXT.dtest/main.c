
// BUILD:  $CC foo.c -dynamiclib  -install_name $RUN_DIR/libfoo-static.dylib  -o $BUILD_DIR/libfoo-static.dylib
// BUILD:  $CC foo.c -dynamiclib  -install_name $RUN_DIR/libfoo-dynamic.dylib -o $BUILD_DIR/libfoo-dynamic.dylib -DDYN
// BUILD:  $CC main.c $BUILD_DIR/libfoo-static.dylib -o $BUILD_DIR/dlsym-RTLD_NEXT.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./dlsym-RTLD_NEXT.exe

#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <mach-o/dyld_priv.h>


// verify RTLD_NEXT search order

int mainSymbol = 4;


// my local implemention of free
void free(void* p) { }


static bool symbolInImage(const char* symName, const char* image)
{
    void* sym = dlsym(RTLD_NEXT, symName);
    if ( sym == NULL )
        return false;
    const char* imagePath = dyld_image_path_containing_address(sym);
    if ( imagePath == NULL )
        return false;
    return (strstr(imagePath, image) != NULL);
}




int main()
{
    printf("[BEGIN] dlsym-RTLD_NEXT\n");

    // verify mainSymbol is not found
    if ( dlsym(RTLD_NEXT, "mainSymbol") != NULL ) {
        printf("[FAIL]  dlsym-RTLD_NEXT: mainSymbol should not have been found\n");
        return 0;
    }

    // verify free is found in OS (not local one)
    if ( !symbolInImage("free", "/usr/lib/") ) {
        printf("[FAIL]  dlsym-RTLD_NEXT: free\n");
        return 0;
    }

    // verify foo is found in libfoo-static.dylib
    if ( !symbolInImage("foo", "libfoo-static.dylib") ) {
        printf("[FAIL]  dlsym-RTLD_NEXT: foo not in libfoo-static.dylib\n");
        return 0;
    }

    void* handle = dlopen(RUN_DIR "/libfoo-dynamic.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        printf("[FAIL]  dlsym-RTLD_NEXT: libfoo-dynamic.dylib could not be loaded\n");
        return 0;
    }

    // verify foo is still found in statically linked lib
    if ( !symbolInImage("foo", "libfoo-static.dylib") ) {
        printf("[FAIL]  dlsym-RTLD_NEXT: foo not in libfoo-static.dylib\n");
        return 0;
    }

    // verify foo2 is not found in libfoo-dynamic.dylib", because RTLD_NEXT only searches thing this image would have seen
    if ( symbolInImage("foo2", "libfoo-dynamic.dylib") ) {
        printf("[FAIL]  dlsym-RTLD_NEXT: foo2 found but should not have been\n");
        return 0;
    }

    printf("[PASS]  dlsym-RTLD_NEXT\n");
	return 0;
}

