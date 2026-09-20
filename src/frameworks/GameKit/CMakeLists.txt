project(GameKit)

remove_sdk_framework(GameKit)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(GameKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/GameKit"
)

add_framework(GameKit
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/GameKit.m

    DEPENDENCIES
        system
        objc
        Foundation
        AppKit
)
