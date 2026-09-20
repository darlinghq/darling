project(AdPlatformsCommon)

remove_sdk_framework(AdPlatformsCommon
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AdPlatformsCommon
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AdPlatformsCommon"
    PRIVATE
)

add_framework(AdPlatformsCommon
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/AdPlatformsCommon.c

    DEPENDENCIES
        system
)
