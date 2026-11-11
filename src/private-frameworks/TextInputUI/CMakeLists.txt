project(TextInputUI)

remove_sdk_framework(TextInputUI
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(TextInputUI
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/TextInputUI"
    PRIVATE
)

add_framework(TextInputUI
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/TextInputUI.c

    DEPENDENCIES
        system
)
