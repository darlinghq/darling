project(CardKit)

remove_sdk_framework(CardKit
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CardKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CardKit"
    PRIVATE
)

add_framework(CardKit
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/CardKit.c

    DEPENDENCIES
        system
)
