project(AppleFSCompression)

remove_sdk_framework(AppleFSCompression
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "119.100.1")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AppleFSCompression
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AppleFSCompression"
    PRIVATE
)

add_framework(AppleFSCompression
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/AppleFSCompression.c

    DEPENDENCIES
        system
)
