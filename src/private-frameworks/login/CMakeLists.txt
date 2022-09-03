project(login)

remove_sdk_framework(login
	PRIVATE
)

add_compile_options(
	-nostdinc
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "137.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(login
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/login"
	PRIVATE
)

add_framework(login
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		functions.c

	DEPENDENCIES
		system
)
