project(IOSurface)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(IOSurface
    FAT
    CURRENT_VERSION
    VERSION "A"

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
