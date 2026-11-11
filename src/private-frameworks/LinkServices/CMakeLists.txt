project(LinkServices)

remove_sdk_framework(LinkServices
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "0.0.0")
set(DYLIB_CURRENT_VERSION "300.6.3")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(LinkServices
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/LinkServices"
    PRIVATE
)

add_framework(LinkServices
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/LinkServices.c

    DEPENDENCIES
        system
)
