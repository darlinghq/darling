project(ColorSyncLegacy)

remove_sdk_framework(ColorSyncLegacy)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ColorSyncLegacy
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ColorSyncLegacy"
)

add_framework(ColorSyncLegacy
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}
    PARENT ApplicationServices

    SOURCES
        src/ColorSyncLegacy.c

    DEPENDENCIES
        system
)
