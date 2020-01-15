project(OpenCL)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(OpenCL
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/OpenCL.m

    DEPENDENCIES
        system
        objc
        Foundation
)
