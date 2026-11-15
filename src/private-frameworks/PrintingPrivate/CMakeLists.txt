project(PrintingPrivate)

remove_sdk_framework(PrintingPrivate
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "251.1.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(PrintingPrivate
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/PrintingPrivate"
    PRIVATE
)

add_framework(PrintingPrivate
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/PrintingPrivate.c

    DEPENDENCIES
        system
)
