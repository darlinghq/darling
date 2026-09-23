project(AOSKit)

remove_sdk_framework(AOSKit
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AOSKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AOSKit"
    PRIVATE
)

add_framework(AOSKit
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/AOSKit.c

    DEPENDENCIES
        system
        CoreFoundation
)
