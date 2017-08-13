project(IOPlatformPluginFamily)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

file(GLOB PPF_C src/*.c)

add_framework(IOPlatformPluginFamily
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		${PPF_C}

	DEPENDENCIES
		system
)
