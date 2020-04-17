
// BUILD:  $CC foo.c -dynamiclib  -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CC main.c $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/dladdr-dylib.exe

// RUN:  ./dladdr-dylib.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <dlfcn.h> 
#include <mach-o/dyld_priv.h>
#if __has_feature(ptrauth_calls)
    #include <ptrauth.h>
#endif

extern void* __dso_handle;

extern void verifyDylib();


static const void* stripPointer(const void* ptr)
{
#if __has_feature(ptrauth_calls)
    return __builtin_ptrauth_strip(ptr, ptrauth_key_asia);
#else
    return ptr;
#endif
}


int bar()
{
    return 2;
}

static int foo()
{
    return 3;
}

__attribute__((visibility("hidden"))) int hide()
{
    return 4;
}

// checks global symbol
static void verifybar()
{
    Dl_info info;
    if ( dladdr(&bar, &info) == 0 ) {
        printf("[FAIL] dladdr(&bar, xx) failed\n");
        exit(0);
    }
    if ( strcmp(info.dli_sname, "bar") != 0 ) {
        printf("[FAIL] dladdr()->dli_sname is \"%s\" instead of \"bar\"\n", info.dli_sname);
        exit(0);
    }
    if ( info.dli_saddr != stripPointer(&bar) ) {
        printf("[FAIL] dladdr()->dli_saddr is not &bar\n");
        exit(0);
    }
    if ( info.dli_fbase != &__dso_handle ) {
        printf("[FAIL] dladdr()->dli_fbase is not image that contains &bar\n");
        exit(0);
    }
}

// checks local symbol
static void verifyfoo()
{
    Dl_info info;
    if ( dladdr(&foo, &info) == 0 ) {
        printf("[FAIL] dladdr(&foo, xx) failed\n");
        exit(0);
    }
    if ( strcmp(info.dli_sname, "foo") != 0 ) {
        printf("[FAIL] dladdr()->dli_sname is \"%s\" instead of \"foo\"\n", info.dli_sname);
        exit(0);
    }
    if ( info.dli_saddr != stripPointer(&foo) ) {
        printf("[FAIL] dladdr()->dli_saddr is not &foo\n");
        exit(0);
    }
    if ( info.dli_fbase != &__dso_handle ) {
        printf("[FAIL] dladdr()->dli_fbase is not image that contains &foo\n");
        exit(0);
    }
}

// checks hidden symbol
static void verifyhide()
{
    Dl_info info;
    if ( dladdr(&hide, &info) == 0 ) {
        printf("[FAIL] dladdr(&hide, xx) failed\n");
        exit(0);
    }
    if ( strcmp(info.dli_sname, "hide") != 0 ) {
        printf("[FAIL] dladdr()->dli_sname is \"%s\" instead of \"hide\"\n", info.dli_sname);
        exit(0);
    }
    if ( info.dli_saddr != stripPointer(&hide) ) {
        printf("[FAIL] dladdr()->dli_saddr is not &hide\n");
        exit(0);
    }
    if ( info.dli_fbase != &__dso_handle ) {
        printf("[FAIL] dladdr()->dli_fbase is not image that contains &hide\n");
        exit(0);
    }
}

// checks dylib symbol
static void verifymalloc()
{
    Dl_info info;
    if ( dladdr(&malloc, &info) == 0 ) {
        printf("[FAIL] dladdr(&malloc, xx) failed\n");
        exit(0);
    }
    if ( strcmp(info.dli_sname, "malloc") != 0 ) {
        printf("[FAIL] dladdr()->dli_sname is \"%s\" instead of \"malloc\"\n", info.dli_sname);
        exit(0);
    }
    if ( info.dli_saddr != stripPointer(&malloc) ) {
        printf("[FAIL] dladdr()->dli_saddr is not &malloc\n");
        exit(0);
    }
    if ( info.dli_fbase != dyld_image_header_containing_address(&malloc) ) {
        printf("[FAIL] dladdr()->dli_fbase is not image that contains &malloc\n");
        exit(0);
    }
}


int main()
{
    printf("[BEGIN] dladdr-dylib\n");
    verifybar();
    verifyhide();
    verifyfoo();
    verifymalloc();

    verifyDylib();

    printf("[PASS] dladdr-dylib\n");
    return 0;
}

