project(AppleSystemInfo)

add_compile_options(
	-nostdinc
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(AppleSystemInfo
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		src/AppleSystemInfo.c
		src/functions.c
	DEPENDENCIES
		system
		${CoreFoundation}
)
