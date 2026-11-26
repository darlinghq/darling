project(VideoToolbox)

remove_sdk_framework(VideoToolbox)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(VideoToolbox
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/VideoToolbox"
)

find_package(FFmpeg REQUIRED)

include_directories(
    ${FFMPEG_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src/CoreAudio
)

add_framework(VideoToolbox
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/VideoToolbox.m
        src/VTCompressionProperties.c
        src/VTDecompressionProperties.c
        src/VTPixelRotationSession.c
        src/VTPixelTransferProperties.c
        src/FigMetalTransferHelper.m
        src/VTSessions.c
        src/VTCompressionSession.c

    DEPENDENCIES
        system
        objc
        Foundation
        CoreMedia
        CoreVideo
        avcodec
        avutil
)

