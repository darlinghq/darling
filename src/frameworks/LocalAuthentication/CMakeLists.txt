project(LocalAuthentication)

add_compile_options(
	-nostdinc
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "240.50.25")

add_framework(LocalAuthentication
	FAT
	CURRENT_VERSION
	VERSION "A"

	SOURCES
		src/LACFSupport.c

	DEPENDENCIES
		system
)
