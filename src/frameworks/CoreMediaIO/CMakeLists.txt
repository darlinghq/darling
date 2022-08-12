project(CoreMediaIO)

remove_sdk_framework(CoreMediaIO)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CoreMediaIO
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CoreMediaIO"
)

add_framework(CoreMediaIO
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/CoreMediaIO.m

    DEPENDENCIES
        system
        objc
        Foundation
)
