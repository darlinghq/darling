project(OpenGL)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(OpenGL
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/OpenGL.c

    DEPENDENCIES
        system
)
