project(IOSurface)

remove_sdk_framework(IOSurface)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(IOSurface
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/IOSurface"
)

add_framework(IOSurface
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/stubs.m
        src/api.m
        src/constants.m
        src/IOSurfaceObjC.m

    DEPENDENCIES
        system
        objc
        Foundation
        IOKit
)
