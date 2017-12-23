project(CoreVideo)

set(DYLIB_COMPAT_VERSION "1.2.0")
set(DYLIB_CURRENT_VERSION "1.5.0")

add_framework(CoreVideo
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/CoreVideo.m

    DEPENDENCIES
        system
        objc
        Foundation
)
