project(AssetCacheServicesExtensions)

remove_sdk_framework(AssetCacheServicesExtensions
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AssetCacheServicesExtensions
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AssetCacheServicesExtensions"
    PRIVATE
)

add_framework(AssetCacheServicesExtensions
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/AssetCacheServicesExtensions.m

    DEPENDENCIES
        system
        objc
        Foundation
)
