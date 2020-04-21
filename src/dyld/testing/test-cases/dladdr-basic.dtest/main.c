
// BUILD:  $CC main.c            -o $BUILD_DIR/dladdr-basic.exe

// RUN:  ./dladdr-basic.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <dlfcn.h> 
#include <mach-o/dyld_priv.h>

extern char** environ;

#if __has_feature(ptrauth_calls)
    #include <ptrauth.h>
#endif

int mydata  = 5;

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

static const void *stripPointer(const void *ptr) {
#if __has_feature(ptrauth_calls)
    return __builtin_ptrauth_strip(ptr, ptrauth_key_asia);
#else
    return ptr;
#endif
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
    if ( info.dli_fbase != dyld_image_header_containing_address(&bar) ) {
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
    if ( info.dli_fbase != dyld_image_header_containing_address(&foo) ) {
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
    if ( info.dli_fbase != dyld_image_header_containing_address(&hide) ) {
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

// checks dylib data symbol
static void verifyenviron()
{
    Dl_info info;
    if ( dladdr(&environ, &info) == 0 ) {
        printf("[FAIL] dladdr(&environ, xx) failed\n");
        exit(0);
    }
    if ( strcmp(info.dli_sname, "environ") != 0 ) {
        printf("[FAIL] dladdr()->dli_sname is \"%s\" instead of \"environ\"\n", info.dli_sname);
        exit(0);
    }
    if ( info.dli_saddr != &environ ) {
        printf("[FAIL] dladdr()->dli_saddr is not &environ\n");
        exit(0);
    }
    if ( info.dli_fbase != dyld_image_header_containing_address(&environ) ) {
        printf("[FAIL] dladdr()->dli_fbase is not image that contains &environ\n");
        exit(0);
    }
}


// checks data symbol in main executable
static void verifymydata()
{
    Dl_info info;
    if ( dladdr(&mydata, &info) == 0 ) {
        printf("[FAIL] dladdr(&mydata, xx) failed\n");
        exit(0);
    }
    if ( strcmp(info.dli_sname, "mydata") != 0 ) {
        printf("[FAIL] dladdr()->dli_sname is \"%s\" instead of \"mydata\"\n", info.dli_sname);
        exit(0);
    }
    if ( info.dli_saddr != &mydata ) {
        printf("[FAIL] dladdr()->dli_saddr is not &mydata\n");
        exit(0);
    }
    if ( info.dli_fbase != dyld_image_header_containing_address(&mydata) ) {
        printf("[FAIL] dladdr()->dli_fbase is not image that contains &mydata\n");
        exit(0);
    }
}


// checks passing NULL for info parameter gracefully fails
static void verifyNULL()
{
    Dl_info info;
    if ( dladdr(&malloc, NULL) != 0 ) {
        printf("[FAIL] dladdr(&malloc, NULL) did not fail\n");
        exit(0);
    }
    if ( dladdr(NULL, NULL) != 0 ) {
        printf("[FAIL] dladdr(NULL, NULL) did not fail\n");
        exit(0);
    }
}

int main()
{
    printf("[BEGIN] dladdr-basic\n");
    verifybar();
    verifyhide();
    verifyfoo();
    verifymalloc();
    verifyenviron();
    verifymydata();
    verifyNULL();

    printf("[PASS] dladdr-basic\n");
    return 0;
}

