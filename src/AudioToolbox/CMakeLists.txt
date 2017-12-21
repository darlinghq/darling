project(AudioToolbox)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(AudioToolbox
	FAT
	CURRENT_VERSION
	VERSION "A"

	SOURCES
		empty.c

	DEPENDENCIES
		system
)
