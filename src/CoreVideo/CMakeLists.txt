project(CoreVideo)

set(DYLIB_COMPAT_VERSION "1.2.0")
set(DYLIB_CURRENT_VERSION "1.5.0")

include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src/external/libcxx/include ${CMAKE_CURRENT_BINARY_DIR})

add_framework(CoreVideo
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
	src/constants.c
	src/stubs.c
        src/CVDisplayLink.mm

    DEPENDENCIES
        system
        Foundation
        CoreGraphics
        cxx
)

