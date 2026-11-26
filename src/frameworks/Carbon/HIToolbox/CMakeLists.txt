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
		src/CarbonEventsCore.c
		src/CarbonEventQueue.c
		src/CarbonEventLoop.c
		src/CarbonEventStream.c
		src/CarbonEvents.c
		src/CarbonEventTargets.c
		src/CarbonApplicationEvents.c
		src/MacApplication.cpp
		src/MacWindows.c
		src/TextInputSources.mm
		src/TextServices.c
		src/IBCarbonRuntime.c
		src/IBCarbonNib.c
		src/IBCarbonWindows.c
		src/Controls.c
		src/Dialogs.c
		src/Menus.c
		src/MenusClassic.c
		src/MenuTracking.m
		src/MouseTracking.c
		src/HITheme.c
		src/constants.c

	DEPENDENCIES
		CoreFoundation
		Foundation
		CoreGraphics
		CoreText
		HIServices
		CoreServices
		QD
		OpenGL
		system
		cxx
)
