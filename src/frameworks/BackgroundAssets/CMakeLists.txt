project(BackgroundAssets)

remove_sdk_framework(BackgroundAssets)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

# The framework exists only for 64-bit processes on macOS (13.0 and later), and the implementation relies on the
# modern Objective-C ABI (ivars in @implementation, property synthesis), so no 32-bit slice is built.
set(DARLING_LIB_64BIT_ONLY TRUE)

generate_sdk_framework(BackgroundAssets
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/BackgroundAssets"
)

add_framework(BackgroundAssets
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/BAConstants.m
        src/BADownload.m
        src/BAURLDownload.m
        src/BAAppExtensionInfo.m
        src/BADownloadManager.m
        src/BAAssetPack.m
        src/BAAssetPackManifest.m

    DEPENDENCIES
        system
        objc
        Foundation
)

# The implementations are written for ARC (no retain, release or dealloc anywhere in the target).
target_compile_options(BackgroundAssets PRIVATE -fobjc-arc)
