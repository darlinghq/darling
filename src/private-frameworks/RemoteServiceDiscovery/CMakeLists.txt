project(RemoteServiceDiscovery)

remove_sdk_framework(RemoteServiceDiscovery
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(RemoteServiceDiscovery
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/RemoteServiceDiscovery"
    PRIVATE
)

add_framework(RemoteServiceDiscovery
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/RemoteServiceDiscovery.c

    DEPENDENCIES
        system
)
