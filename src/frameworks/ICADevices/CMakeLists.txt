project(ICADevices)

remove_sdk_framework(ICADevices)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ICADevices
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ICADevices"
)

add_framework(ICADevices
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/ICADevices.m

    DEPENDENCIES
        system
        objc
        Foundation
)
