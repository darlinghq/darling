
// BUILD:  $CC foo.c -dynamiclib  -install_name $RUN_DIR/libfoo-static.dylib  -o $BUILD_DIR/libfoo-static.dylib
// BUILD:  $CC foo.c -dynamiclib  -install_name $RUN_DIR/libfoo-dynamic.dylib -o $BUILD_DIR/libfoo-dynamic.dylib -DDYN
// BUILD:  $CC main.c $BUILD_DIR/libfoo-static.dylib -o $BUILD_DIR/dlsym-RTLD_MAIN_ONLY.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./dlsym-RTLD_MAIN_ONLY.exe

#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <mach-o/dyld_priv.h>


// verify RTLD_MAIN_ONLY search order

int mainSymbol = 4;


// my local implemention of free
void free(void* p) { }


static bool symbolInImage(const char* symName, const char* image)
{
    void* sym = dlsym(RTLD_MAIN_ONLY, symName);
    if ( sym == NULL )
        return false;
    const char* imagePath = dyld_image_path_containing_address(sym);
    if ( imagePath == NULL )
        return false;
    return (strstr(imagePath, image) != NULL);
}




int main()
{
    printf("[BEGIN] dlsym-RTLD_MAIN_ONLY\n");

    // verify mainSymbol is found
    if ( !symbolInImage("mainSymbol", "dlsym-RTLD_MAIN_ONLY") ) {
        printf("[FAIL]  dlsym-RTLD_MAIN_ONLY: mainSymbol should have been found\n");
        return 0;
    }

    // verify free is found in this program - not in OS
    if ( !symbolInImage("free", "dlsym-RTLD_MAIN_ONLY") ) {
        printf("[FAIL]  dlsym-RTLD_MAIN_ONLY: free\n");
        return 0;
    }

    // verify foo is not found
    if ( dlsym(RTLD_MAIN_ONLY, "foo") != NULL ) {
        printf("[FAIL]  dlsym-RTLD_MAIN_ONLY: foo should not have been found\n");
        return 0;
    }

    void* handle = dlopen(RUN_DIR "/libfoo-dynamic.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        printf("[FAIL]  dlsym-RTLD_MAIN_ONLY: libfoo-dynamic.dylib could not be loaded\n");
        return 0;
    }

    // verify foo is still not found
    if ( dlsym(RTLD_MAIN_ONLY, "foo") != NULL ) {
        printf("[FAIL]  dlsym-RTLD_MAIN_ONLY: foo should not have been found after dlopen\n");
        return 0;
    }

    // verify foo2 is not found in libfoo-dynamic.dylib", because RTLD_MAIN_ONLY only searches main executable
    if ( dlsym(RTLD_MAIN_ONLY, "foo2") != NULL ) {
        printf("[FAIL]  dlsym-RTLD_MAIN_ONLY: foo2 found but should not have been\n");
        return 0;
    }

    printf("[PASS]  dlsym-RTLD_MAIN_ONLY\n");
	return 0;
}

