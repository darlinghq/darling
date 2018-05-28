project(OpenGL)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

include(darling_framework)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_EGL egl)

include_directories(${PC_EGL_INCLUDE_DIRS})

add_framework(OpenGL
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        OpenGL.c

    DEPENDENCIES
        system
        GL
        EGL
)

set_property(TARGET OpenGL APPEND_STRING PROPERTY LINK_FLAGS
	" -sub_library libGL")
