project(LangAnalysis)

remove_sdk_framework(LangAnalysis)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(LangAnalysis
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/LangAnalysis"
)

add_framework(LangAnalysis
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}
    PARENT ApplicationServices

    SOURCES
        src/LangAnalysis.c

    DEPENDENCIES
        system
)
