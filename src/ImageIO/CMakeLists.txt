project(ImageIO)

add_compile_options(
	-nostdinc
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_framework(ImageIO
	FAT
	CURRENT_VERSION
	VERSION "A"

	SOURCES
		functions.c

	DEPENDENCIES
		system
)
