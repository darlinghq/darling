project(login)

add_compile_options(
	-nostdinc
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_framework(login
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		functions.c

	DEPENDENCIES
		system
)
