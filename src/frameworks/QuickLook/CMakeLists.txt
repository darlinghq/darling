project(QuickLook)

remove_sdk_framework(QuickLook)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "0.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(QuickLook
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/QuickLook"
)

add_framework(QuickLook
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/QuickLook.m
        src/QLTestingLog.m
        src/QLDebugController.m
        src/QLDebugServerController.m
        src/QLDebugSatelliteController.m
        src/QLPreview.m
        src/QLImageWrapper.m
	    constants.c

    DEPENDENCIES
        system
        objc
        Foundation
	CoreFoundation
)
