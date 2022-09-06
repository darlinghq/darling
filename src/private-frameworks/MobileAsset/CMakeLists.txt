project(MobileAsset)

remove_sdk_framework(MobileAsset
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(MobileAsset
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/MobileAsset"
	PRIVATE
)

add_framework(MobileAsset
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/MobileAsset.m
        src/MAProgressHandler.m
        src/MAAsset.m
        src/MAAssetQuery.m
        src/ASAsset.m
        src/ASAssetQuery.m
        src/MAXpcManager.m
        src/MAAbsoluteAssetId.m
        src/MADownloadOptions.m
        src/MAMsuDownloadOptions.m
        src/MADownloadConfig.m
        src/MAProgressNotification.m
        src/MAXpcConnection.m

    DEPENDENCIES
        system
        objc
        Foundation
)
