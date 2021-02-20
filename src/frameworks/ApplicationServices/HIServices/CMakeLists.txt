project(HIServices)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "628.0.0")

add_framework(HIServices
    FAT
    CURRENT_VERSION
    VERSION "A"
    PARENT ApplicationServices

    SOURCES
        src/HIServices.c
        src/Processes.cpp

    DEPENDENCIES
	    CoreFoundation
        system
        cxx
        LaunchServices
        CoreGraphics
)
