project(SafeEjectGPU)

remove_sdk_framework(SafeEjectGPU
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(SafeEjectGPU
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/SafeEjectGPU"
    PRIVATE
)

add_framework(SafeEjectGPU
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/SafeEjectGPU.m

    DEPENDENCIES
        system
        objc
        Foundation
)
