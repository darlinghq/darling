project(CoreCDP)

remove_sdk_framework(CoreCDP
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CoreCDP
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CoreCDP"
    PRIVATE
)

add_framework(CoreCDP
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/CoreCDP.c

    DEPENDENCIES
        system
)
