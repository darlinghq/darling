project(CoreFollowUp)

remove_sdk_framework(CoreFollowUp
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "0.0.0")
set(DYLIB_CURRENT_VERSION "281.5.2")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CoreFollowUp
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CoreFollowUp"
    PRIVATE
)

add_framework(CoreFollowUp
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/CoreFollowUp.c

    DEPENDENCIES
        system
)
