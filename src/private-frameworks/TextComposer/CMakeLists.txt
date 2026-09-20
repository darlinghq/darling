project(TextComposer)

remove_sdk_framework(TextComposer
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(TextComposer
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/TextComposer"
    PRIVATE
)

add_framework(TextComposer
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/TextComposer.c

    DEPENDENCIES
        system
)
