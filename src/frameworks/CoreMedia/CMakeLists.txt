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
        src/CMTimeRange.c
        src/CMTimeMapping.c
        src/CMSampleBuffer.c
        src/CMCore.c
        src/CMAttachment.c
        src/CMTimebase.c
        src/CMSimpleQueue.c
        src/CMMemoryPool.c

    DEPENDENCIES
        system
        Foundation
        CoreVideo
)
