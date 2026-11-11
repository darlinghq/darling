project(CoreHaptics)

remove_sdk_framework(CoreHaptics)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CoreHaptics
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CoreHaptics"
)

# The framework exists only for 64-bit processes on macOS, so no 32-bit slice is built.
set(DARLING_LIB_64BIT_ONLY TRUE)

add_framework(CoreHaptics
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/CoreHaptics.m

    DEPENDENCIES
        system
        objc
        Foundation
)
