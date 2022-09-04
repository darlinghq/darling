project(HIServices)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "628.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(HIServices
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/HIServices"
)

add_framework(HIServices
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}
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
