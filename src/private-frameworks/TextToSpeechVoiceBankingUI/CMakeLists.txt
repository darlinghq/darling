project(TextToSpeechVoiceBankingUI)

remove_sdk_framework(TextToSpeechVoiceBankingUI
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(TextToSpeechVoiceBankingUI
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/TextToSpeechVoiceBankingUI"
    PRIVATE
)

add_framework(TextToSpeechVoiceBankingUI
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/TextToSpeechVoiceBankingUI.c

    DEPENDENCIES
        system
)
