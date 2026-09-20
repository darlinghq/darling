project(NotificationCenter)

remove_sdk_framework(NotificationCenter)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1553.6.1")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(NotificationCenter
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/NotificationCenter"
)

add_framework(NotificationCenter
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/NotificationCenter.c

    DEPENDENCIES
        system
)
