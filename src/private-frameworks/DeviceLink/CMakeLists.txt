project(DeviceLink)

set(DYLIB_COMPAT_VERSION "100.0.0")
set(DYLIB_CURRENT_VERSION "264.0.0")

include_directories(include)

add_framework(DeviceLink
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		src/DeviceLink.c

	DEPENDENCIES
		system
)
