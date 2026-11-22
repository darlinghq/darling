project(NaturalLanguage)

remove_sdk_framework(NaturalLanguage)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "114.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(NaturalLanguage
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/NaturalLanguage"
)

add_framework(NaturalLanguage
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/NLConstants.m
        src/NLTokenizer.m
        src/NLTagger.m
        src/NLLanguageRecognizer.m
        src/NLEmbedding.m

    DEPENDENCIES
        system
        objc
        Foundation
        CoreFoundation
)
