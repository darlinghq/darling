project(AppleFSCompression)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "119.100.1")

add_framework(AppleFSCompression
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION "A"

    SOURCES
        src/AppleFSCompression.c

    DEPENDENCIES
        system
)
