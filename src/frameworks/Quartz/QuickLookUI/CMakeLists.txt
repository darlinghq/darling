project(QuickLookUI)

set(FRAMEWORK_VERSION "A")

remove_sdk_framework(QuickLookUI)

# GUESS: not documented; copied from Quartz. dyld refuses a client that records a higher
# minimum compatibility version than this.
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

generate_sdk_framework(QuickLookUI
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/QuickLookUI"
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_framework(QuickLookUI
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/QLPreviewItem.m
		src/QLPreviewPanel.m
		src/QLPreviewView.m

	DEPENDENCIES
		system
		objc
		Foundation
		AppKit
)
