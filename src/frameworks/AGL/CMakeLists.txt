project(AGL)

set(DYLIB_COMPAT_VERSION "2.0.0")
set(DYLIB_CURRENT_VERSION "3.3.2")

add_framework(AGL
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/AGL.c

    DEPENDENCIES
    	CoreFoundation
    	Carbon
        system
)
