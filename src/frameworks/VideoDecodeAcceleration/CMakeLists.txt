project(VideoDecodeAcceleration)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(VideoDecodeAcceleration
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/VideoDecodeAcceleration.c

    DEPENDENCIES
        system
)
