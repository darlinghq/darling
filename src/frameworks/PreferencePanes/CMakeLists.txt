project(PreferencePanes)

remove_sdk_framework(PreferencePanes)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "16.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(PreferencePanes
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/PreferencePanes"
)

add_framework(PreferencePanes
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/NSPreferencePane.m

    DEPENDENCIES
        system
        objc
        Foundation
        AppKit
)
target_include_directories(PreferencePanes PRIVATE include)
