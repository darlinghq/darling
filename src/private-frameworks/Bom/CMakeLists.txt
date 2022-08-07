project(Bom)

set(DYLIB_COMPAT_VERSION "2.0.0")
set(DYLIB_CURRENT_VERSION "195.0.0")

add_framework(Bom
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION "A"

    SOURCES
        src/Bom.c

    DEPENDENCIES
        system
)
