project(ApplicationServices)

set(DYLIB_CURRENT_VERSION "48.0.0")

add_framework(ApplicationServices
	FAT CURRENT_VERSION
	VERSION "A"
	SOURCES
		ApplicationServices.c
	DEPENDENCIES
		system CoreServices CoreFoundation
	LINK_FLAGS
		" -Wl,-reexport_library,${CMAKE_BINARY_DIR}/src/CoreServices/CoreServices \
		  -Wl,-reexport_library,${CMAKE_BINARY_DIR}/src/external/cocotron/CoreGraphics/CoreGraphics"
)

add_dependencies(ApplicationServices CoreServices CoreGraphics)
