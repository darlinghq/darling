project(DebugSymbols)

add_compile_options(
	-nostdinc
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "137.0.0")

add_framework(DebugSymbols
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		functions.c
		copy.c

	DEPENDENCIES
		system
)
