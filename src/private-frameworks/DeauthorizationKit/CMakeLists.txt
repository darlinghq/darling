project(DeauthorizationKit)

remove_sdk_framework(DeauthorizationKit
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(DeauthorizationKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/DeauthorizationKit"
    PRIVATE
)

add_framework(DeauthorizationKit
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/DeauthorizationKit.c

    DEPENDENCIES
        system
)
