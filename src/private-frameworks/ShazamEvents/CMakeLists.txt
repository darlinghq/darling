project(ShazamEvents)

remove_sdk_framework(ShazamEvents
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ShazamEvents
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/ShazamEvents"
)


add_framework(ShazamEvents
	PRIVATE
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/_TtC12ShazamEvents21LiveMusicEventFetcher.m
		src/SHShazamEventsServiceInterfaces.m
		src/_TtC12ShazamEvents24LiveMusicEventConnection.m
		src/ShazamEvents.m

	DEPENDENCIES
		system
		objc
		Foundation
)