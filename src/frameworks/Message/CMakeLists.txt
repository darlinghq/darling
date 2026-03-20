project(Message)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1604.0.0")
set(FRAMEWORK_VERSION "A")

remove_sdk_framework(Message
)

generate_sdk_framework(Message
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/Message"
)

add_framework(Message
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/Message.m

    DEPENDENCIES
        system
        objc
        Foundation
)
