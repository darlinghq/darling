project(CoreGraphics)

set(DYLIB_COMPAT_VERSION "64.0.0")
set(DYLIB_CURRENT_VERSION "1265.9.0")

add_framework(CoreGraphics
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/CoreGraphics.m
        src/CGVirtualDisplayMode.m
        src/CGVirtualDisplayDescriptor.m
        src/CGVirtualDisplaySettings.m
        src/CGVirtualDisplay.m

    DEPENDENCIES
        system
        objc
        Foundation
)
