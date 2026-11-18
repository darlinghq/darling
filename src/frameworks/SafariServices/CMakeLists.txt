project(SafariServices)

remove_sdk_framework(SafariServices)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(SafariServices
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/SafariServices"
)

add_framework(SafariServices
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/SFUniversalLink.m

    DEPENDENCIES
        system
        objc
        Foundation
)
