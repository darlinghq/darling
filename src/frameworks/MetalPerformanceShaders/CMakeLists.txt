project(MetalPerformanceShaders)

remove_sdk_framework(MetalPerformanceShaders)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "122.5.6")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(MetalPerformanceShaders
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/MetalPerformanceShaders"
)

add_framework(MetalPerformanceShaders
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/MetalPerformanceShaders.m

    DEPENDENCIES
        system
        objc
        Foundation
)
