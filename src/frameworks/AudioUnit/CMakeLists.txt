project(AudioUnit)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(AudioUnit
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/AudioUnit.c

    DEPENDENCIES
        system
)
