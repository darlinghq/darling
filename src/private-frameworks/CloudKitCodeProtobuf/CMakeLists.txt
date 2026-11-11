project(CloudKitCodeProtobuf)

remove_sdk_framework(CloudKitCodeProtobuf
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "2360.120.2")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CloudKitCodeProtobuf
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CloudKitCodeProtobuf"
    PRIVATE
)

add_framework(CloudKitCodeProtobuf
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/CloudKitCodeProtobuf.c

    DEPENDENCIES
        system
)
