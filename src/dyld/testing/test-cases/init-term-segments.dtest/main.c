

// BUILD:  $CC foo.c -dynamiclib -fno-register-global-dtors-with-atexit -Wl,-segprot,__SOMETEXT,rx,rx -Wl,-segprot,__MORETEXT,rx,rx -o $BUILD_DIR/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/init-term-segments.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./init-term-segments.exe

#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>

extern bool foo(bool* ptr);

int main()
{
    printf("[BEGIN] init-term-segments\n");

    void* h = dlopen(RUN_DIR "/libfoo.dylib", RTLD_NOW);
    if (h == NULL) {
		printf("[FAIL]  init-term-segments - dlerror = %s\n", dlerror());
		return 0;
    }

    void* fooSym = dlsym(RTLD_DEFAULT, "foo");
    if ( fooSym == NULL ) {
		printf("[FAIL]  init-term-segments - dlsym failure\n");
		return 0;
    }

    bool ranTerm = false;
    bool ranInit = ((__typeof(&foo))fooSym)(&ranTerm);
    if (!ranInit) {
		printf("[FAIL]  init-term-segments - didn't run init\n");
		return 0;
    }

    if ( dlclose(h) != 0 ) {
		printf("[FAIL]  init-term-segments - didn't dlclose\n");
		return 0;
    }

    if (!ranTerm) {
		printf("[FAIL]  init-term-segments - didn't run term\n");
		return 0;
    }

    printf("[PASS]  init-term-segments\n");
	return 0;
}

