project(NewsAnalyticsUpload)

remove_sdk_framework(NewsAnalyticsUpload
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(NewsAnalyticsUpload
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/NewsAnalyticsUpload"
    PRIVATE
)

add_framework(NewsAnalyticsUpload
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/NewsAnalyticsUpload.c

    DEPENDENCIES
        system
)
