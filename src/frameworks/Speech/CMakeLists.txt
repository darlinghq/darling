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

add_separated_framework(Speech
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

# SpeechTranscriber is macOS 26 Swift API; the Swift runtime and overlays exist for 64-bit slices only.
if (SWIFT_TOOLCHAIN_FOUND AND BUILD_TARGET_64BIT)
    if (NOT TARGET swiftFoundation)
        message(FATAL_ERROR "Speech requires swiftFoundation")
    endif ()
    _darling_swift_resource_dir(swift_resource_dir)
    file(MAKE_DIRECTORY "${DARLING_SWIFT_MODULE_CACHE}")
    set(swift_src "${CMAKE_CURRENT_SOURCE_DIR}/src/SpeechTranscriber.swift")

    foreach (arch IN LISTS DARLING_ARCHS_64BIT)
        set(module_dir "${DARLING_SWIFT_MODULE_DIR}/${arch}")
        file(MAKE_DIRECTORY "${module_dir}")
        set(swift_arch_flags)
        if (arch STREQUAL "arm64")
            if (NOT DARLING_ARM64_CPU)
                message(FATAL_ERROR "DARLING_ARM64_CPU must name the common ARM64 CPU baseline")
            endif ()
            list(APPEND swift_arch_flags -target-cpu "${DARLING_ARM64_CPU}")
        endif ()

        _darling_swift_compile("Speech.${arch}" swift_objects_${arch}
            OBJECT "${CMAKE_CURRENT_BINARY_DIR}/SpeechTranscriber.${arch}.swift.o"
            MODULE "${module_dir}/Speech.swiftmodule"
            SOURCES "${swift_src}"
            DEPENDS swiftFoundation "${module_dir}/Foundation.swiftmodule"
            COMMON
                -target "${arch}-apple-macosx${DARLING_SWIFT_DEPLOYMENT_TARGET}"
                -sdk "${DARLING_SWIFT_SDK}"
                -resource-dir "${swift_resource_dir}"
                -I "${DARLING_SWIFT_SOURCE_DIR}/stdlib-modules"
                -I "${module_dir}"
                -module-cache-path "${DARLING_SWIFT_MODULE_CACHE}"
                -swift-version 5
                -module-name Speech
                -enable-library-evolution
                -parse-as-library
                -O
                -define-availability "SwiftStdlib 5.7:macOS 13.0"
                ${DARLING_SWIFT_SDK_SUBFRAMEWORKS}
                ${swift_arch_flags}
            COMMENT "Compiling SpeechTranscriber.swift for ${arch}")
        set_source_files_properties(${swift_objects_${arch}} PROPERTIES EXTERNAL_OBJECT TRUE GENERATED TRUE)
        target_sources(Speech_${arch} PRIVATE ${swift_objects_${arch}})
        set_property(TARGET Speech_${arch} APPEND_STRING PROPERTY LINK_FLAGS
            " -mmacosx-version-min=${DARLING_SWIFT_DEPLOYMENT_TARGET} -Wl,-L,${DARLING_SWIFT_SOURCE_DIR} -Wl,-L,${CMAKE_BINARY_DIR}/src/external/swift")
        target_link_libraries(Speech_${arch} PRIVATE
            "${DARLING_SWIFT_SOURCE_DIR}/libswiftCore.dylib"
            swiftFoundation)
    endforeach ()
endif ()
