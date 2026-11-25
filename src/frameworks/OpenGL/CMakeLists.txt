project(OpenGL)

remove_sdk_framework(OpenGL)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

include(darling_framework)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_EGL egl)

include_directories(${PC_EGL_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/src/startup/mldr/elfcalls)

generate_sdk_framework(OpenGL
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/OpenGL"
)

add_framework(OpenGL
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        OpenGL.c
        gl_exports.c
        gl_trampolines.c

    DEPENDENCIES
        system
        GL
        GLU
        CoreFoundation
        CoreGraphics
        IOSurface
        IOKit
)

# OpenGL.c loads the native EGL at run time instead of linking it.
add_dependencies(OpenGL EGL)

set_property(TARGET OpenGL APPEND_STRING PROPERTY LINK_FLAGS
    " -Wl,-reexport_library,${CMAKE_BINARY_DIR}/src/native/libGL.dylib \
    -Wl,-reexport_library,${CMAKE_BINARY_DIR}/src/native/libGLU.dylib ")
InstallSymlink(Versions/Current/Libraries
			"${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/OpenGL.framework/Libraries")

# Libraries/libGL.dylib. On macOS this library holds the gl* entry points and OpenGL.framework
# re-exports it. Here the entry points that hold the thread's binding of the current context
# before they call the host are in the framework, so the direction is the other way round: this
# library re-exports the framework (the host's libGL is behind a private wrapper,
# /usr/lib/native/libGL.dylib, which the framework re-exports in turn). An application that loads
# libGL.dylib from the Libraries directory and looks functions up there thereby reaches the same
# entry points as one that links the framework. Same install name and versions as before.
set(DYLIB_INSTALL_NAME "/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
add_darling_library(OpenGLLibGL SHARED libGL.c)
set_target_properties(OpenGLLibGL PROPERTIES OUTPUT_NAME GL
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Libraries")
target_link_libraries(OpenGLLibGL PRIVATE system)
make_fat(OpenGLLibGL)
reexport(OpenGLLibGL OpenGL ${CMAKE_CURRENT_BINARY_DIR}/OpenGL)
install(TARGETS OpenGLLibGL
    DESTINATION libexec/darling/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries)
