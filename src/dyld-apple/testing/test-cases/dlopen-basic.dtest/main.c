
// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/test.dylib
// BUILD:  $CC foo.c -bundle     -o $BUILD_DIR/test.bundle
// BUILD:  $CC main.c            -o $BUILD_DIR/dlopen-basic.exe

// RUN:  ./dlopen-basic.exe

#include <stdio.h>
#include <dlfcn.h>


static void tryImage(const char* path)
{
    printf("[BEGIN] dlopen-basic %s\n", path);
	void* handle = dlopen(path, RTLD_LAZY);
	if ( handle == NULL ) {
        printf("dlerror(): %s\n", dlerror());
        printf("[FAIL] dlopen-basic %s\n", path);
		return;
	}
	
	void* sym = dlsym(handle, "foo");
	if ( sym == NULL ) {
        printf("dlerror(): %s\n", dlerror());
        printf("[FAIL] dlopen-basic %s\n", path);
		return;
	}
	
	int result = dlclose(handle);
	if ( result != 0 ) {
        printf("dlclose() returned %c\n", result);
        printf("[FAIL] dlopen-basic %s\n", path);
		return;
	}

    printf("[PASS] dlopen-basic %s\n", path);
}



int main()
{
	tryImage("test.bundle");
	tryImage("test.dylib");
  
	return 0;
}

