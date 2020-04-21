
// BUILD:  $CC main.c  -o $BUILD_DIR/sdk-check.exe

// RUN:  ./sdk-check.exe

#include <stdio.h>
#include <string.h>
#include <mach-o/dyld_priv.h>

extern struct mach_header __dso_handle;

int main()
{
    printf("[BEGIN] dyld_get_sdk_version\n");

    // should succeed
    if ( dyld_get_sdk_version(&__dso_handle) == 0 ) {
        printf("[FAIL] dyld_get_sdk_version: expected SDK\n");
        return 0;
    }

    // should fail
    const char* text = "bad text";
    if ( dyld_get_sdk_version((struct mach_header*)text) != 0 ) {
        printf("[FAIL] dyld_get_sdk_version: expected failure\n");
        return 0;
    }


#if TARGET_OS_WATCH
    uint32_t iosVersion = dyld_get_program_sdk_version();
    uint32_t watchOSVersion = dyld_get_program_sdk_watch_os_version();
    if (iosVersion != (watchOSVersion + 0x00070000)) {
        printf("[FAIL] dyld_get_program_sdk_watch_os_version\n");
        return 0;
    }
#endif
#if TARGET_OS_BRIDGE
    uint32_t iosVersion = dyld_get_program_sdk_version();
    uint32_t bridgeOSVersion = dyld_get_program_sdk_bridge_os_version();
    if (bridgeOSVersion != (watchOSVersion + 0x00090000)) {
        printf("[FAIL] dyld_get_program_sdk_watch_os_version\n");
        return 0;
    }
#endif
    printf("[PASS] dyld_get_sdk_version\n");
	return 0;
}

