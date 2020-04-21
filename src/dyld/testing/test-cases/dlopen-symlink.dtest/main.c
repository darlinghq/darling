
// BUILD:  $CC foo.c -dynamiclib -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:  cd $BUILD_DIR && ln -s libfoo.dylib libfoo-symlink.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-symlink.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./dlopen-symlink.exe


#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <mach-o/dyld_priv.h>


int main()
{
    printf("[BEGIN] dlopen-symlink\n");

    // call dlopen() with a path that is a symlink
    void* handle = dlopen(RUN_DIR "/libfoo-symlink.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        printf("dlerror(): %s\n", dlerror());
        printf("[FAIL]  dlopen-symlink\n");
        return 0;
    }

    // walk images to see if path was converted to real path
    const char* foundPath = NULL;
    int count = _dyld_image_count();
    for (int i=0; i < count; ++i) {
        const char* path = _dyld_get_image_name(i);
        //printf("path[%2d]=%s\n", i, path);
        if ( strstr(path, "libfoo") != NULL ) {
            if ( foundPath == NULL ) {
                foundPath = path;
            }
            else {
                printf("[FAIL]  dlopen-symlink: more than one libfoo found\n");
                return 0;
            }
        }
    }
    if ( foundPath == NULL ) {
        printf("[FAIL]  dlopen-symlink: no libfoo found\n");
        return 0;
    }
    if ( strstr(foundPath, "libfoo-symlink") != NULL ) {
        printf("[FAIL]  dlopen-symlink: path is symlink not real path: %s\n", foundPath);
        return 0;
    }

    printf("[PASS]  dlopen-symlink\n");
	return 0;
}

