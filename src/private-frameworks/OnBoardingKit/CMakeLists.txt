set(FRAMEWORK_VERSION "A")

generate_sdk_framework(OnBoardingKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/OnBoardingKit"
    PRIVATE
)

# The framework exists only for 64-bit processes on macOS, so no 32-bit slice is built.
set(DARLING_LIB_64BIT_ONLY TRUE)

add_framework(OnBoardingKit
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/OnBoardingKit.m

    DEPENDENCIES
        system
        objc
        Foundation
        AppKit
)
