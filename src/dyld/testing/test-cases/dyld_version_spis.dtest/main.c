
// BUILD:  $CC main.c  -o $BUILD_DIR/versions.exe

// RUN:  ./versions.exe

#include <stdio.h>
#include <string.h>
#include <mach-o/dyld_priv.h>

extern struct mach_header __dso_handle;

int main()
{
    printf("[BEGIN] dyld_version_spi\n");
    dyld_platform_t active = dyld_get_active_platform();
    dyld_platform_t base = dyld_get_base_platform(active);
    dyld_build_version_t absoluteMin = { .platform = base, .version = 0 };
    dyld_build_version_t absoluteMax = { .platform = base, .version = 0xffffffff };

#if TARGET_OS_OSX
    if ( base != PLATFORM_MACOS ) {
        printf("[FAIL] base.platform %u incorrect for macOS\n", base);
        return 0;
    }
#elif TARGET_OS_IOS
    if ( base != PLATFORM_IOS ) {
        printf("[FAIL] base.platform %u incorrect for iOS\n", base);
        return 0;
    }
#elif TARGET_OS_TV
    if ( base != PLATFORM_TVOS ) {
        printf("[FAIL] base.platform %u incorrect for tvOS\n", base);
        return 0;
    }
#elif TARGET_OS_BRIDGE
    if ( base != PLATFORM_BRIDGEOS ) {
        printf("[FAIL] base.platform %u incorrect for wacthOS\n", base);
        return 0;
    }
#elif TARGET_OS_WATCH
    if ( base != PLATFORM_WATCHOS ) {
        printf("[FAIL] base.platform %u incorrect for bridgeOS\n", base);
        return 0;
    }
#else
    printf("[FAIL] Running on unknown platform\n");
    return 0;
#endif

#if TARGET_OS_SIMULATOR
    if (dyld_is_simulator_platform(active) != true) {
        printf("[FAIL] active platform %u should be a simulator\n", active);
        return 0;
    }
#else
    if (dyld_is_simulator_platform(active) == true) {
        printf("[FAIL] active platform %u should not be a simulator\n", active);
        return 0;
    }
#endif

    if (dyld_is_simulator_platform(base) == true) {
        printf("[FAIL] base platform %u should not be a simulator\n", base);
        return 0;
    }

    if (!dyld_sdk_at_least(&__dso_handle, absoluteMin)) {
        printf("[FAIL] executable sdk version should not < 1.0.0\n");
        return 0;
    }

    if (dyld_sdk_at_least(&__dso_handle, absoluteMax)) {
        printf("[FAIL] executable sdk version should not > 65536.0.0\n");
        return 0;
    }

    if (!dyld_minos_at_least(&__dso_handle, absoluteMin)) {
        printf("[FAIL] executable min version should not < 1.0.0\n");
        return 0;
    }

    if (dyld_minos_at_least(&__dso_handle, absoluteMax)) {
        printf("[FAIL] executable min version should not > 65536.0.0\n");
        return 0;
    }

    printf("[PASS] dyld_version_spi\n");
    return 0;
}

