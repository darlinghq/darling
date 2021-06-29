project(HIToolbox)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "994.6.0")

add_framework(HIToolbox
	FAT
	CURRENT_VERSION
	VERSION "A"
	PARENT "Carbon"

	SOURCES
		src/Keyboards.cpp
		src/Events.cpp
		src/MacWindows.cpp
		src/TextInputSources.mm
		src/constants.c

	DEPENDENCIES
		CoreFoundation
		Foundation
		system
)
