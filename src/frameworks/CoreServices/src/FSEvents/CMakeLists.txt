set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1239.200.12")
add_framework(FSEvents
	FAT
	CURRENT_VERSION
	VERSION "A"
	PARENT "CoreServices"
	SOURCES
		FSEvents.m
		FSEventsImpl.m
	DEPENDENCIES
		CoreFoundation
		Foundation
		system
)
