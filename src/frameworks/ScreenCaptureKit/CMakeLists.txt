project(ScreenCaptureKit)

remove_sdk_framework(ScreenCaptureKit)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ScreenCaptureKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ScreenCaptureKit"
)

add_framework(ScreenCaptureKit
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/ScreenCaptureKit.m

    DEPENDENCIES
        system
        objc
        Foundation
)
