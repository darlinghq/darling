project(SpeechSynthesis)

remove_sdk_framework(SpeechSynthesis)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(SpeechSynthesis
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/SpeechSynthesis"
)

add_framework(SpeechSynthesis
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}
    PARENT ApplicationServices

    SOURCES
        src/SpeechSynthesis.c

    DEPENDENCIES
        system
)
