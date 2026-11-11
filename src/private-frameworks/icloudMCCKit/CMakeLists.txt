project(icloudMCCKit)

remove_sdk_framework(icloudMCCKit
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "0.0.0")
set(DYLIB_CURRENT_VERSION "0.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(icloudMCCKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/icloudMCCKit"
    PRIVATE
)

add_framework(icloudMCCKit
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/icloudMCCKit.c

    DEPENDENCIES
        system
)
