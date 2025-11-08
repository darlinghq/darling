project(LocalAuthentication)

remove_sdk_framework(LocalAuthentication)

add_compile_options(
	-nostdinc
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "240.50.25")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(LocalAuthentication
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/LocalAuthentication"
)

add_framework(LocalAuthentication
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/LACFSupport.c
		src/LAContext.m
		src/stubs.c

	DEPENDENCIES
		system
		Foundation
)
