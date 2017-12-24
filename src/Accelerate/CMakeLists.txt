project(Accelerate)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "4.0.0")

add_framework(Accelerate
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/Accelerate.c

    DEPENDENCIES
        system
)
