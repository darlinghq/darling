project(DiskImages)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")

set(DYLIB_COMPAT_VERSION "1.0.8")

add_framework(DiskImages
	PRIVATE
	VERSION "A"
	CURRENT_VERSION
	SOURCES
		functions.c
	DEPENDENCIES
		system
)

