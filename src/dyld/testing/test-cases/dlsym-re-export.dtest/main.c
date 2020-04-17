
// BUILD:  mkdir -p $BUILD_DIR/sub1 $BUILD_DIR/sub2
// BUILD:  $CC sub1.c -dynamiclib -install_name @rpath/libsub1.dylib   -o $BUILD_DIR/sub1/libsub1.dylib
// BUILD:  $CC sub2.c -dynamiclib -install_name @rpath/libsub2.dylib   -o $BUILD_DIR/sub2/libsub2.dylib
// BUILD:  $CC foo.c  -dynamiclib -install_name $RUN_DIR/libfoo.dylib  -o $BUILD_DIR/libfoo.dylib -rpath @loader_path/sub1 -Wl,-reexport_library,$BUILD_DIR/sub1/libsub1.dylib -Wl,-reexport_library,$BUILD_DIR/sub2/libsub2.dylib
// BUILD:  $CC main.c  -o $BUILD_DIR/dlsym-reexport.exe -DRUN_DIR="$RUN_DIR" -rpath @loader_path/sub2

// RUN:  ./dlsym-reexport.exe

// rpath for sub1 is found in libfoo.dylib.  rpath for sub2 is found in dlsym-reexport.exe

#include <stdio.h>
#include <dlfcn.h>

int main()
{
    printf("[BEGIN] dlsym-re-export\n");
    // RTLD_FIRST means dlsym() should only search libfoo.dylib (and any re-exports)
	void* handle = dlopen(RUN_DIR "/libfoo.dylib", RTLD_FIRST);
	if ( handle == NULL ) {
        printf("dlerror(): %s\n", dlerror());
        printf("[FAIL]  dlsym-re-export\n");
		return 0;
	}

	void* sym1 = dlsym(handle, "sub1");
	if ( sym1 == NULL ) {
        printf("dlerror(): %s\n", dlerror());
        printf("[FAIL]  dlsym-re-export\n");
		return 0;
	}

	void* sym2 = dlsym(handle, "sub2");
	if ( sym2 == NULL ) {
        printf("dlerror(): %s\n", dlerror());
        printf("[FAIL]  dlsym-re-export\n");
		return 0;
	}

    printf("[PASS]  dlsym-re-export\n");
	return 0;
}

