project(VideoDecodeAcceleration)

remove_sdk_framework(VideoDecodeAcceleration)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(VideoDecodeAcceleration
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/VideoDecodeAcceleration"
)

add_framework(VideoDecodeAcceleration
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/VideoDecodeAcceleration.c

    DEPENDENCIES
        system
)
