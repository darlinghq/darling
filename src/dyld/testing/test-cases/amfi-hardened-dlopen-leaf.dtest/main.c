// BUILD_ONLY: MacOSX

// BOOT_ARGS: dyld_flags=2

// BUILD:  $CC my.c -dynamiclib -o $BUILD_DIR/libmy.dylib -install_name $RUN_DIR/libmy.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/amfi-hardened-dlopen-leaf.exe -DHARDENED=1
// BUILD:  $CC main.c -o $BUILD_DIR/amfi-not-hardened-dlopen-leaf.exe

// RUN:  DYLD_AMFI_FAKE=0x14 ./amfi-hardened-dlopen-leaf.exe
// RUN:  DYLD_AMFI_FAKE=0x3F ./amfi-not-hardened-dlopen-leaf.exe


#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <dlfcn.h>
#include <mach/host_info.h>
#include <mach/mach.h>
#include <mach/mach_host.h>

void tryPath(const char* prog, const char* path)
{
    void* handle = dlopen(path, RTLD_LAZY);
#if HARDENED
    if ( handle != NULL ) {
        printf("[FAIL]  %s dlopen(%s) unexpectedly succeeded\n", prog, path);
        exit(0);
    }
#else
    if ( handle == NULL ) {
        printf("[FAIL]  %s dlopen(%s) - %s\n", prog, path, dlerror());
        exit(0);
    }
#endif

}

int main(int arg, const char* argv[])
{
    printf("[BEGIN] %s\n", argv[0]);

    // verify leaf name leads to dylib in /usr/lib/
    void* handle = dlopen("libc.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        printf("[FAIL]  %s dlopen - %s\n", argv[0], dlerror());
        return 0;
    }

    // verify file system relative paths: hardened should fail
    tryPath(argv[0], "libmy.dylib");
    tryPath(argv[0], "./libmy.dylib");
    tryPath(argv[0], "../amfi-hardened-dlopen-leaf/libmy.dylib");

    printf("[PASS]  %s\n", argv[0]);

	return 0;
}



