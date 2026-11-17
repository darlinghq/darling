project(OpenGLES)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

# iOS's OpenGL ES: EAGLContext on the host's EGL, and the GL functions, each
# the host's (Mesa's on Linux, the phone driver's on Android). See
# src/EAGLContext.m. OpenGLES exists only for 64-bit processes (x86_64 and
# arm64), as iOS apps are, so no 32-bit slice is built.
set(DARLING_LIB_64BIT_ONLY TRUE)

generate_sdk_framework(OpenGLES VERSION A HEADER "include/OpenGLES")

add_framework(OpenGLES
    IOSSUPPORT
    CURRENT_VERSION
    VERSION A
    SOURCES
        src/EAGLContext.m
        src/gles-functions.c
    DEPENDENCIES
        Foundation
        CoreGraphics
        QuartzCore
        objc
        system
)
target_include_directories(OpenGLES PRIVATE include src ${CMAKE_SOURCE_DIR}/src/startup/mldr/elfcalls)
target_compile_options(OpenGLES PRIVATE -fobjc-arc)
