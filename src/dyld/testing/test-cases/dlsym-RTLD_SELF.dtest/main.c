
// BUILD:  $CC foo.c -dynamiclib  -install_name $RUN_DIR/libfoo-static.dylib  -o $BUILD_DIR/libfoo-static.dylib
// BUILD:  $CC foo.c -dynamiclib  -install_name $RUN_DIR/libfoo-dynamic.dylib -o $BUILD_DIR/libfoo-dynamic.dylib -DDYN
// BUILD:  $CC main.c $BUILD_DIR/libfoo-static.dylib -o $BUILD_DIR/dlsym-RTLD_SELF.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./dlsym-RTLD_SELF.exe

#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <mach-o/dyld_priv.h>


// verify RTLD_SELF search order

int mainSymbol = 4;


// my local implemention of free
void free(void* p) { }


static bool symbolInImage(const char* symName, const char* image)
{
    void* sym = dlsym(RTLD_SELF, symName);
    if ( sym == NULL )
        return false;
    const char* imagePath = dyld_image_path_containing_address(sym);
    if ( imagePath == NULL )
        return false;
    return (strstr(imagePath, image) != NULL);
}




int main()
{
    printf("[BEGIN] dlsym-RTLD_SELF\n");

    // verify mainSymbol is found
    if ( dlsym(RTLD_SELF, "mainSymbol") == NULL ) {
        printf("[FAIL]  dlsym-RTLD_SELF: mainSymbol should have been found\n");
        return 0;
    }

    // verify free is found in this program - not in OS
    if ( !symbolInImage("free", "dlsym-RTLD_SELF") ) {
        printf("[FAIL]  dlsym-RTLD_SELF: free\n");
        return 0;
    }

    // verify foo is found in libfoo-static.dylib
    if ( !symbolInImage("foo", "libfoo-static.dylib") ) {
        printf("[FAIL]  dlsym-RTLD_SELF: foo not in libfoo-static.dylib\n");
        return 0;
    }

    void* handle = dlopen(RUN_DIR "/libfoo-dynamic.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        printf("[FAIL]  dlsym-RTLD_SELF: libfoo-dynamic.dylib could not be loaded\n");
        return 0;
    }

    // verify foo is still found in statically linked lib
    if ( !symbolInImage("foo", "libfoo-static.dylib") ) {
        printf("[FAIL]  dlsym-RTLD_SELF: foo not in libfoo-static.dylib\n");
        return 0;
    }

    // verify foo2 is not found in libfoo-dynamic.dylib", because RTLD_SELF only searches thing this image would have seen
    if ( symbolInImage("foo2", "libfoo-dynamic.dylib") ) {
        printf("[FAIL]  dlsym-RTLD_SELF: foo2 found but should not have been\n");
        return 0;
    }

    printf("[PASS]  dlsym-RTLD_SELF\n");
	return 0;
}

