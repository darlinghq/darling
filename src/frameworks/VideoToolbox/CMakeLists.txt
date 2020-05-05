project(VideoToolbox)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(VideoToolbox
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/VideoToolbox.m
        src/FigMetalTransferHelper.m

    DEPENDENCIES
        system
        objc
        Foundation
)
