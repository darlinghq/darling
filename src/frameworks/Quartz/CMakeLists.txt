project(Quartz)

remove_sdk_framework(Quartz)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/QuickLookUI/include
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(Quartz
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/Quartz"
)

add_framework(Quartz
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/Quartz.c
	    QuickLookUI/src/QLPreviewPanel.m
	    QuickLookUI/src/QLPreviewView.m

    DEPENDENCIES
        system
	objc
	Foundation
	AppKit
)

add_dependencies(Quartz QuickLook)

# Ideally would be done through a nested QuickLookUI framework
set_property(TARGET Quartz APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-reexport_library,${CMAKE_BINARY_DIR}/src/frameworks/QuickLook/QuickLook ")
