project(TimeSync)

remove_sdk_framework(TimeSync
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(TimeSync
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/TimeSync"
    PRIVATE
)

add_framework(TimeSync
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/TimeSync.c

    DEPENDENCIES
        system
)
