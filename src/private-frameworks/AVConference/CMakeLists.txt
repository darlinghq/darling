project(AVConference)

remove_sdk_framework(AVConference
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AVConference
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AVConference"
    PRIVATE
)

add_framework(AVConference
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/AVConference.c

    DEPENDENCIES
        system
)
