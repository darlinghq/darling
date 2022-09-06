project(CoreSymbolication)

remove_sdk_framework(CoreSymbolication
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "62046.0.0")

file(GLOB CS_C src/*.c)

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CoreSymbolication
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CoreSymbolication"
	PRIVATE
)

add_framework(CoreSymbolication
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		${CS_C}

	DEPENDENCIES
		system
)
