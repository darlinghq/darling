project(NLP)

remove_sdk_framework(NLP
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "233.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(NLP
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/NLP"
    PRIVATE
)

add_framework(NLP
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/NLP.c

    DEPENDENCIES
        system
)
