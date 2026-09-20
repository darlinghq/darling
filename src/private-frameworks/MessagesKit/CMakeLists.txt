project(MessagesKit)

remove_sdk_framework(MessagesKit
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "800.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(MessagesKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/MessagesKit"
    PRIVATE
)

add_framework(MessagesKit
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/MessagesKit.c

    DEPENDENCIES
        system
)
