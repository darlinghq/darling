project(ColorSync)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "4.7.0")

add_framework(ColorSync
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/ColorSync.c

    DEPENDENCIES
        system
)
