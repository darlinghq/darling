project(QuickLook)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "0.0.0")

add_framework(QuickLook
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/QuickLook.m
        src/QLTestingLog.m
        src/QLDebugController.m
        src/QLDebugServerController.m
        src/QLDebugSatelliteController.m
        src/QLPreview.m
        src/QLImageWrapper.m

    DEPENDENCIES
        system
        objc
        Foundation
)
