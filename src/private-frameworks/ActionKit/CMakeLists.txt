project(ActionKit)

remove_sdk_framework(ActionKit
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ActionKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ActionKit"
    PRIVATE
)

add_framework(ActionKit
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/ActionKit.c

    DEPENDENCIES
        system
)
