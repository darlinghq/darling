project(ImageIO)

add_compile_options(
	-nostdinc
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(ImageIO
	FAT
	CURRENT_VERSION
	VERSION "A"

	SOURCES
		functions.c

	DEPENDENCIES
		system
)
