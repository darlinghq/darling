project(VideoToolbox)

remove_sdk_framework(VideoToolbox)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(VideoToolbox
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/VideoToolbox"
)

add_framework(VideoToolbox
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/VideoToolbox.m
        src/FigMetalTransferHelper.m

    DEPENDENCIES
        system
        objc
        Foundation
)
