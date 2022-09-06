project(AssetCacheServices)

remove_sdk_framework(AssetCacheServices
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "106.1.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AssetCacheServices
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AssetCacheServices"
    PRIVATE
)

add_framework(AssetCacheServices
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/AssetCacheServices.m
        src/ACSURLSession.m
        src/ACSURLSessionUploadTask.m
        src/ACSURLSessionDataTask.m
        src/ACSURLSessionTask.m
        src/ACSURLSessionDownloadTask.m

    DEPENDENCIES
        system
        objc
        Foundation
)
