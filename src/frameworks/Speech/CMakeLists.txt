project(Speech)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")

remove_sdk_framework(Speech
)

generate_sdk_framework(Speech
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/Speech"
)

add_framework(Speech
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/Speech.m
        src/SFAcousticFeature.m
        src/SFVoiceAnalytics.m
        src/_SFSearchResult.m
        src/SFSpeechRecognitionTask.m
        src/_SFSpeechRecognitionBlockTask.m
        src/_SFSpeechRecognitionDelegateTask.m
        src/SFSpeechRecognitionResult.m
        src/SFSpeechRecognizer.m
        src/SFTranscriptionSegment.m
        src/_SFSearchRequest.m
        src/SFSpeechRecognitionRequest.m
        src/SFSpeechURLRecognitionRequest.m
        src/SFSpeechAudioBufferRecognitionRequest.m
        src/SFTranscription.m

    DEPENDENCIES
        system
        objc
        Foundation
)
