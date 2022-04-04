project(MediaAccessibility)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "62.0.0")

add_framework(MediaAccessibility
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/MediaAccessibility.c

    DEPENDENCIES
        system
)
