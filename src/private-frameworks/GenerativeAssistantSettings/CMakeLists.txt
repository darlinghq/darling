project(GenerativeAssistantSettings)

remove_sdk_framework(GenerativeAssistantSettings
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "3530.1.1")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(GenerativeAssistantSettings
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/GenerativeAssistantSettings"
    PRIVATE
)

add_framework(GenerativeAssistantSettings
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/GenerativeAssistantSettings.c

    DEPENDENCIES
        system
)
