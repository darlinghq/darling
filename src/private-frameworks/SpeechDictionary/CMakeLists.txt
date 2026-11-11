set(FRAMEWORK_VERSION "A")

generate_sdk_framework(SpeechDictionary
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/SpeechDictionary"
    PRIVATE
)

add_framework(SpeechDictionary
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/SpeechDictionary.m

    DEPENDENCIES
        system
        objc
        Foundation
)
