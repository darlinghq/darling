project(MobileDevice)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(MobileDevice
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION "A"

    SOURCES
        src/MobileDevice.m
        src/AMSupportURLConnectionDelegate.m
        src/AMSupportURLSession.m
        src/RPStream.m
        src/PersonalizedImage.m

    DEPENDENCIES
        system
        objc
        Foundation
)
