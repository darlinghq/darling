project(vecLib)

# TODO: Move to /src/CMakeLists.txt when done
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vMisc/include)

add_subdirectory(vMisc)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "671.250.4")

add_framework(vecLib
    FAT
    CURRENT_VERSION
    VERSION "A"
    PARENT "Accelerate"

    SOURCES
        empty.c

    DEPENDENCIES
        system
)
reexport(vecLib vMisc)

InstallSymlink(Accelerate.framework/Versions/A/Frameworks/vecLib.framework ${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/vecLib.framework)
