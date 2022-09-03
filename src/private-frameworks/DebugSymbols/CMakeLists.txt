project(DebugSymbols)

remove_sdk_framework(DebugSymbols
	PRIVATE
)

add_compile_options(
	-nostdinc
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "137.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(DebugSymbols
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/DebugSymbols"
	PRIVATE
)

add_framework(DebugSymbols
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		functions.c
		DebugSymbols.c

	DEPENDENCIES
		system
		CoreFoundation
)
