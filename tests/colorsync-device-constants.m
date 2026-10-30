// ColorSync device dictionary keys and device class constants regression test.
// Build as a C/Objective-C executable linked against CoreFoundation and ColorSync:
//   clang -framework CoreFoundation -framework ColorSync tests/colorsync-device-constants.m -o /tmp/colorsync-test
// Run it with `darling shell`. Exits non-zero on failure.
#include <CoreFoundation/CoreFoundation.h>
#include <ColorSync/ColorSync.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    char buf[128];
    int failures = 0;

    struct {
        const char *name;
        const CFStringRef *val;
        const char *expected;
    } checks[] = {
        { "kColorSyncDeviceProfileURL", &kColorSyncDeviceProfileURL, "DeviceProfileURL" },
        { "kColorSyncDeviceModeDescriptions", &kColorSyncDeviceModeDescriptions, "DeviceModeDescriptions" },
        { "kColorSyncDeviceDefaultProfileID", &kColorSyncDeviceDefaultProfileID, "DeviceDefaultProfileID" },
        { "kColorSyncDeviceDescriptions", &kColorSyncDeviceDescriptions, "DeviceDescriptions" },
        { "kColorSyncFactoryProfiles", &kColorSyncFactoryProfiles, "FactoryProfiles" },
        { "kColorSyncDeviceUserScope", &kColorSyncDeviceUserScope, "DeviceUserScope" },
        { "kColorSyncDeviceHostScope", &kColorSyncDeviceHostScope, "DeviceHostScope" },
        { "kColorSyncPrinterDeviceClass", &kColorSyncPrinterDeviceClass, "prnt" },
        { "kColorSyncCustomProfiles", &kColorSyncCustomProfiles, "CustomProfiles" },
        { "kColorSyncDisplayDeviceClass", &kColorSyncDisplayDeviceClass, "mntr" },
    };

    int n = sizeof(checks) / sizeof(checks[0]);
    for (int i = 0; i < n; i++) {
        if (!checks[i].val || !*checks[i].val) {
            printf("FAIL: %s is NULL\n", checks[i].name);
            failures++;
            continue;
        }
        if (!CFStringGetCString(*checks[i].val, buf, sizeof(buf), kCFStringEncodingUTF8)) {
            printf("FAIL: %s could not be converted to C string\n", checks[i].name);
            failures++;
            continue;
        }
        if (strcmp(buf, checks[i].expected) != 0) {
            printf("FAIL: %s is '%s', expected '%s'\n", checks[i].name, buf, checks[i].expected);
            failures++;
            continue;
        }
    }

    if (failures == 0) {
        printf("PASS: %d ColorSync device constants verified\n", n);
    } else {
        printf("FAIL: %d/%d ColorSync device constants failed\n", failures, n);
    }
    return failures != 0;
}
