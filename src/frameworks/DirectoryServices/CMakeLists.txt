project(DirectoryService)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(DirectoryService
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/DirectoryService.c

    DEPENDENCIES
        system
)
