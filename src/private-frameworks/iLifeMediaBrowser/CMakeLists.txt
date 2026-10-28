project(iLifeMediaBrowser)

remove_sdk_framework(iLifeMediaBrowser
	PRIVATE
)

add_compile_options(
	-nostdinc
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(iLifeMediaBrowser
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/iLifeMediaBrowser"
	PRIVATE
)

add_framework(iLifeMediaBrowser
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}
	SOURCES
		src/iLifeMediaBrowser.m
	DEPENDENCIES
		Foundation
)
