project(Suggestions)

remove_sdk_framework(Suggestions
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(Suggestions
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/Suggestions"
    PRIVATE
)

add_framework(Suggestions
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/Suggestions.c

    DEPENDENCIES
        system
)
