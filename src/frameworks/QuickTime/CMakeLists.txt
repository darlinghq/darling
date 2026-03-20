project(QuickTime)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "3039.0.0")
set(FRAMEWORK_VERSION "A")

remove_sdk_framework(QuickTime
)

generate_sdk_framework(QuickTime
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/QuickTime"
)

add_framework(QuickTime
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/QuickTime.c

    DEPENDENCIES
        system
)
