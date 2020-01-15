project(ColorSyncLegacy)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(ColorSyncLegacy
    FAT
    CURRENT_VERSION
    VERSION "A"
    PARENT ApplicationServices

    SOURCES
        src/ColorSyncLegacy.c

    DEPENDENCIES
        system
)
