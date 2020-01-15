project(CoreAudio)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(CoreAudio
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/CoreAudio.m

    DEPENDENCIES
        system
        objc
        Foundation
)
