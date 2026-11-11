project(FinderSync)

remove_sdk_framework(FinderSync)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1865.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(FinderSync
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/FinderSync"
)

add_framework(FinderSync
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/FinderSync.m

    DEPENDENCIES
        system
        objc
        Foundation
        AppKit
)
target_include_directories(FinderSync PRIVATE include)
