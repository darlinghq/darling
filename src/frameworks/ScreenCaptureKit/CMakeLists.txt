project(ScreenCaptureKit)

remove_sdk_framework(ScreenCaptureKit)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ScreenCaptureKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ScreenCaptureKit"
)

# The framework exists only for 64-bit processes on macOS, so no 32-bit slice is built.
set(DARLING_LIB_64BIT_ONLY TRUE)

add_framework(ScreenCaptureKit
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/ScreenCaptureKit.m

    DEPENDENCIES
        system
        objc
        Foundation
)
