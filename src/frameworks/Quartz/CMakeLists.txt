project(Quartz)

set(FRAMEWORK_VERSION "A")

remove_sdk_framework(Quartz)

get_path_preframework(quartz_sdk_path)
append_path_sdk_subframework(${quartz_sdk_path} quartz_sdk_path Quartz
	VERSION ${FRAMEWORK_VERSION}
)

add_subdirectory(ImageKit)
add_subdirectory(QuartzComposer)
add_subdirectory(QuartzFilters)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

generate_sdk_framework(Quartz
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/Quartz"
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/QuickLookUI/include
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
		QuickLook
		ImageKit
)

set(QuickLook_BUILD ${CMAKE_BINARY_DIR}/src/frameworks/QuickLook/QuickLook)
set(ImageKit_BUILD ${CMAKE_CURRENT_BINARY_DIR}/ImageKit/ImageKit)
set(QuartzComposer_BUILD ${CMAKE_CURRENT_BINARY_DIR}/QuartzComposer/QuartzComposer)
set(QuartzFilters_BUILD ${CMAKE_CURRENT_BINARY_DIR}/QuartzFilters/QuartzFilters)
set(PDFKit_BUILD ${CMAKE_BINARY_DIR}/src/frameworks/PDFKit/PDFKit)
if (COMPONENT_gui)
	set(QuartzCore_BUILD ${CMAKE_BINARY_DIR}/src/external/cocotron/QuartzCore/QuartzCore)
else()
	set(QuartzCore_BUILD ${CMAKE_BINARY_DIR}/src/frameworks/dev-stubs/QuartzCore/QuartzCore)
endif()

# Ideally would be done through a nested QuickLookUI framework
reexport(Quartz QuickLook ${QuickLook_BUILD})
reexport(Quartz ImageKit ${ImageKit_BUILD})
reexport(Quartz QuartzComposer ${QuartzComposer_BUILD})
reexport(Quartz QuartzFilters ${QuartzFilters_BUILD})
reexport(Quartz PDFKit ${PDFKit_BUILD})
reexport(Quartz QuartzCore ${QuartzCore_BUILD})
