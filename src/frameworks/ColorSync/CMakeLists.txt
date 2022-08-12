project(ColorSync)

remove_sdk_framework(ColorSync)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "4.7.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ColorSync
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ColorSync"
)

add_framework(ColorSync
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/ColorSync.c

    DEPENDENCIES
        CoreFoundation
        system
)
