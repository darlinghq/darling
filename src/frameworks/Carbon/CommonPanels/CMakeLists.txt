project(CommonPanels)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "101.0.0")

add_framework(CommonPanels
	FAT
	CURRENT_VERSION
	VERSION "A"
	PARENT "Carbon"

	SOURCES
		src/CommonPanels.m

	DEPENDENCIES
		system
		objc
		Foundation
)
