project(MetalPerformanceShaders)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "122.5.6")

add_framework(MetalPerformanceShaders
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/MetalPerformanceShaders.m

    DEPENDENCIES
        system
        objc
        Foundation
)
