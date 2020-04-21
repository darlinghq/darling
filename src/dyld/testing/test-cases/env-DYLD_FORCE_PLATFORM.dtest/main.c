
// BUILD_ONLY: MacOSX
// BUILD:  $CC main.c            -o $BUILD_DIR/DYLD_FORCE_PLATFORM.exe -DENABLE_ALT_PLATFORMS=1 -ldarwintest
// BUILD:  $CC main.c            -o $BUILD_DIR/DYLD_FORCE_PLATFORM_FAIL.exe -ldarwintest
// BUILD:  $TASK_FOR_PID_ENABLE  $BUILD_DIR/DYLD_FORCE_PLATFORM.exe
// BUILD:  $TASK_FOR_PID_ENABLE  $BUILD_DIR/DYLD_FORCE_PLATFORM_FAIL.exe

// RUN:  ./DYLD_FORCE_PLATFORM.exe
// RUN:  ./DYLD_FORCE_PLATFORM_FAIL.exe

#include <mach-o/dyld_priv.h>
#include "dyld_test.h"

#if ENABLE_ALT_PLATFORMS
__attribute__((section("__DATA,__allow_alt_plat"))) uint64_t dummy;
T_DECL_DYLD(DYLD_FORCE_PLATFORM, "Test that DYLD_FORCE_PLATFORM works correctly", T_META_ENVVAR("DYLD_FORCE_PLATFORM=6")) {
    dyld_build_version_t ios12 = { .platform = PLATFORM_IOSMAC, .version = 0x000c0000 };
    T_EXPECT_EQ_UINT(dyld_get_active_platform(), PLATFORM_IOSMAC, "dyld_get_active_platform() should return PLATFORM_IOSMAC");

    // libswiftUIKit.dylib exists in /System/iOSSupport/usr/lib/swift
    // We should be able to dlopen it only if we are correctly prepending the /System/iOSSupport root path
    T_EXPECT_TRUE(dlopen_preflight("/usr/lib/swift/libswiftUIKit.dylib"), "Should be able to dlopen libswiftUIKit but %s", dlerror());
#if 0
    //FIXME: This has to be disabled until we can fix rdar://47156760
    T_EXPECT_TRUE(dyld_program_minos_at_least(ios12), "DYLD_FORCE_PLATFORM should synthesize an iOS min version greater than 12.0");
    T_EXPECT_TRUE(dyld_program_sdk_at_least(ios12), "DYLD_FORCE_PLATFORM should synthesize an iOS sdk versio greater than 12.0");
#endif
}
#else
T_DECL_DYLD(DYLD_FORCE_PLATFORM_FAIL, "Test that DYLD_FORCE_PLATFORM fails correctly", T_META_ENVVAR("DYLD_FORCE_PLATFORM=6")) {
    T_EXPECT_EQ_UINT(dyld_get_active_platform(), PLATFORM_MACOS, "dyld_get_active_platform() should return PLATFORM_IOSMAC");

    // libswiftUIKit.dylib exists in /System/iOSSupport/usr/lib/swift
    // We should not be able to dlopen this as we don't expect to find it in a macOS location.  If it starts
    // being in a macOS location then we should update this test
    T_EXPECT_FALSE(dlopen_preflight("/usr/lib/swift/libswiftUIKit.dylib"), "Should not be able to dlopen libswiftUIKit");
}
#endif
