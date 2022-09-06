project(ExceptionHandling)

remove_sdk_framework(ExceptionHandling)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "13.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ExceptionHandling
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ExceptionHandling"
)

add_framework(ExceptionHandling
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/ExceptionHandler.m

    DEPENDENCIES
        system
        objc
        Foundation
)
