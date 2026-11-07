project(IOSurface)

remove_sdk_framework(IOSurface)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(IOSurface
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/IOSurface"
)

# dmabuf.c calls into the host C library through the ELF loader.
include_directories(${CMAKE_SOURCE_DIR}/src/startup/mldr/elfcalls)

add_framework(IOSurface
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/stubs.m
        src/api.m
        src/constants.m
        src/IOSurfaceObjC.m
        src/dmabuf.c

    DEPENDENCIES
        system
        objc
        Foundation
        IOKit
)
