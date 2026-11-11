project(MediaPlayer)

remove_sdk_framework(MediaPlayer)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(MediaPlayer
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/MediaPlayer"
)

add_framework(MediaPlayer
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/MediaPlayer.m

    DEPENDENCIES
        system
        objc
        Foundation
        AppKit
)
target_include_directories(MediaPlayer PRIVATE include)
