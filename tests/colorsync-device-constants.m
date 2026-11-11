// ColorSync device dictionary keys, device classes, profile keys and transform option
// keys regression test. Expected values are those observed on macOS (see the commit message).
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
        { "kColorSyncPrinterDeviceClass", &kColorSyncPrinterDeviceClass, "prtr" },
        { "kColorSyncCustomProfiles", &kColorSyncCustomProfiles, "CustomProfiles" },
        { "kColorSyncDisplayDeviceClass", &kColorSyncDisplayDeviceClass, "mntr" },
        { "kColorSyncGenericGrayGamma22Profile", &kColorSyncGenericGrayGamma22Profile, "com.apple.ColorSync.GenericGrayGamma2.2" },
        { "kColorSyncGenericLabProfile", &kColorSyncGenericLabProfile, "com.apple.ColorSync.GenericLab" },
        { "kColorSyncGenericXYZProfile", &kColorSyncGenericXYZProfile, "com.apple.ColorSync.GenericXYZ" },
        { "kColorSyncGenericCMYKProfile", &kColorSyncGenericCMYKProfile, "com.apple.ColorSync.GenericCMYK" },
        { "kColorSyncSRGBProfile", &kColorSyncSRGBProfile, "com.apple.ColorSync.sRGB" },
        { "kColorSyncAdobeRGB1998Profile", &kColorSyncAdobeRGB1998Profile, "com.apple.ColorSync.AdobeRGB1998" },
        { "kColorSyncITUR2020Profile", &kColorSyncITUR2020Profile, "com.apple.ColorSync.ITUR2020" },
        { "kColorSyncProfileHeader", &kColorSyncProfileHeader, "com.apple.ColorSync.ProfileHeader" },
        { "kColorSyncProfileDescription", &kColorSyncProfileDescription, "com.apple.ColorSync.ProfileDescription" },
        { "kColorSyncProfileURL", &kColorSyncProfileURL, "com.apple.ColorSync.ProfileURL" },
        { "kColorSyncProfileMD5Digest", &kColorSyncProfileMD5Digest, "com.apple.ColorSync.ProfileMD5Digest" },
        { "kColorSyncProfile", &kColorSyncProfile, "ColorSyncProfile" },
        { "kColorSyncRenderingIntent", &kColorSyncRenderingIntent, "ColorSyncRenderingIntent" },
        { "kColorSyncTransformTag", &kColorSyncTransformTag, "ColorSyncTransformTag" },
        { "kColorSyncTransformSimplifiedConversionData", &kColorSyncTransformSimplifiedConversionData, "com.apple.cmm.SimplifiedConversion" },
        { "kColorSyncConversion3DLut", &kColorSyncConversion3DLut, "com.apple.cmm.3D-LUT" },
        { "kColorSyncConversionGridPoints", &kColorSyncConversionGridPoints, "com.apple.cmm.GridPointCount" },
        { "kColorSyncBlackPointCompensation", &kColorSyncBlackPointCompensation, "ColorSyncBlackPointCompensation" },
        { "kColorSyncExtendedRange", &kColorSyncExtendedRange, "com.apple.cmm.ExtendedRange" },
        { "kColorSyncDoNotSubstituteProfiles", &kColorSyncDoNotSubstituteProfiles, "com.apple.cmm.DoNotSubstituteProfiles" },
        { "kColorSyncWaitForCacheReply", &kColorSyncWaitForCacheReply, "com.apple.ColorSync.WaitForCacheReply" },
        { "kColorSyncGainMapBaselineHeadroom", &kColorSyncGainMapBaselineHeadroom, "com.apple.ColorSync.GainMapBaselineHeadroom" },
        { "kColorSyncMonoGainMapParams", &kColorSyncMonoGainMapParams, "com.apple.ColorSync.MonoGainMapParams" },
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
