set(FRAMEWORK_VERSION "A")

generate_sdk_framework(SpeechObjects
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/SpeechObjects"
    PRIVATE
)

add_framework(SpeechObjects
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/SpeechObjects.m

    DEPENDENCIES
        system
        objc
        Foundation
)
