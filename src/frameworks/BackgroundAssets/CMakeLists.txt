project(BackgroundAssets)

remove_sdk_framework(BackgroundAssets)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(BackgroundAssets
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/BackgroundAssets"
)

add_framework(BackgroundAssets
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/BackgroundAssets.m

    DEPENDENCIES
        system
        objc
        Foundation
)
