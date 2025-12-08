project(CoreMedia)

remove_sdk_framework(CoreMedia)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CoreMedia
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CoreMedia"
)

add_framework(CoreMedia
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/CoreMedia.c
        src/CMTime.c
        src/CMSampleBuffer.c

    DEPENDENCIES
        system
        Foundation
)
