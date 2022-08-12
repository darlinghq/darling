project(OpenAL)

remove_sdk_framework(OpenAL)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(OpenAL
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/OpenAL"
)

add_framework(OpenAL
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/OpenAL.c

    DEPENDENCIES
        system
)
