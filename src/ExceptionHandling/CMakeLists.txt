project(ExceptionHandling)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "13.0.0")

add_framework(ExceptionHandling
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/ExceptionHandler.m

    DEPENDENCIES
        system
        objc
        Foundation
)
