project(AssetCacheServices)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "106.1.0")

add_framework(AssetCacheServices
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION "A"

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
