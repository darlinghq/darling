project(ForceFeedback)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.2")

add_framework(ForceFeedback
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/ForceFeedback.c

    DEPENDENCIES
        system
)
