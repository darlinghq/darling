project(HIToolbox)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "994.6.0")

set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(HIToolbox
	BASE_PATH ${carbon_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "include/HIToolbox"
)

add_framework(HIToolbox
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "Carbon"

	SOURCES
		src/Keyboards.cpp
		src/Events.cpp
		src/MacWindows.cpp
		src/TextInputSources.mm
		src/IBCarbonRuntime.c
		src/constants.c

	DEPENDENCIES
		CoreFoundation
		Foundation
		system
)
