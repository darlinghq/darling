project(Timeline)

remove_sdk_framework(Timeline
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(Timeline
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/Timeline"
    PRIVATE
)

add_framework(Timeline
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/Timeline.c

    DEPENDENCIES
        system
)
