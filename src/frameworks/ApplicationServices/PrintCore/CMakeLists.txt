project(PrintCore)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "503.8.0")

add_framework(PrintCore
    FAT
    CURRENT_VERSION
    VERSION "A"
    PARENT ApplicationServices

    SOURCES
        src/PrintCore.m
        src/PMInkChecker.m

    DEPENDENCIES
        system
        objc
        Foundation
)
