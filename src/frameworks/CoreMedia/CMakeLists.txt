project(CoreMedia)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(CoreMedia
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/CoreMedia.c
	src/CMTime.c

    DEPENDENCIES
        system
        Foundation
)
