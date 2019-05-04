project(DrawSprocket)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(DrawSprocket
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/DrawSprocket.c

    DEPENDENCIES
    	CoreFoundation
    	Carbon
        system
)
