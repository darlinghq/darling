/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * arm64 only: libobjc exports objc_retain_xN and objc_release_xN for
 * x0-x15 and x19-x28 (as on macOS, per the SDK's libobjc.tbd), and
 * objc_msgSend_stret is not exported on arm64. Each entry point retains or
 * releases the object passed in xN.
 *
 * Build: clang -arch arm64 -fno-objc-arc retain_release_xn.m -lobjc -framework Foundation
 */
#include <dlfcn.h>
#include <stdio.h>
#include <objc/runtime.h>
#import <Foundation/Foundation.h>

static int failures;

static void check(const char *what, int cond)
{
    printf("%s: %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond)
        failures++;
}

static const int regs[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                            19, 20, 21, 22, 23, 24, 25, 26, 27, 28 };

int main(void)
{
#if __arm64__
    char name[32];
    int missing = 0;
    for (unsigned i = 0; i < sizeof(regs) / sizeof(regs[0]); i++) {
        snprintf(name, sizeof(name), "objc_retain_x%d", regs[i]);
        missing += dlsym(RTLD_DEFAULT, name) == NULL;
        snprintf(name, sizeof(name), "objc_release_x%d", regs[i]);
        missing += dlsym(RTLD_DEFAULT, name) == NULL;
    }
    check("all 52 entry points are exported", missing == 0);
    check("no entry point for x16", dlsym(RTLD_DEFAULT, "objc_retain_x16") == NULL);
    check("objc_msgSend_stret is not exported on arm64", dlsym(RTLD_DEFAULT, "objc_msgSend_stret") == NULL);

    NSObject *obj = [[NSObject alloc] init];
    NSUInteger before = [obj retainCount];
    id result;

    /* objc_retain_x19 takes the object in x19 and returns it in x0. */
    __asm__ volatile(
        "mov x19, %1\n\t"
        "bl _objc_retain_x19\n\t"
        "mov %0, x0"
        : "=r"(result) : "r"(obj)
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10",
          "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x19", "x30", "memory");
    check("objc_retain_x19 returns the object", result == obj);
    check("objc_retain_x19 retains", [obj retainCount] == before + 1);

    __asm__ volatile(
        "mov x19, %0\n\t"
        "bl _objc_release_x19"
        : : "r"(obj)
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10",
          "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x19", "x30", "memory");
    check("objc_release_x19 releases", [obj retainCount] == before);
    [obj release];
#else
    printf("SKIP: arm64 only\n");
#endif
    return failures != 0;
}
