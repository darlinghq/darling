project(OpenCL)

remove_sdk_framework(OpenCL)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(OpenCL
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/OpenCL"
)

add_framework(OpenCL
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/OpenCL.m

    DEPENDENCIES
        system
        objc
        Foundation
)
