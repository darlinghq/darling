project(PrintCore)

remove_sdk_framework(PrintCore)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "503.8.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(PrintCore
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/PrintCore"
)

add_framework(PrintCore
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}
    PARENT ApplicationServices

    SOURCES
        src/PrintCore.m
        src/PMInkChecker.m

    DEPENDENCIES
        system
        objc
        Foundation
)
