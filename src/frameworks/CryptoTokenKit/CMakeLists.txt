project(CryptoTokenKit)

remove_sdk_framework(CryptoTokenKit)

add_compile_options(
	-nostdinc
	-fblocks
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CryptoTokenKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CryptoTokenKit"
)

add_framework(CryptoTokenKit
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/ctkclient.c
		src/ctkclient_test.c
		src/ctkloginhelper.c

	DEPENDENCIES
		system
		CoreFoundation
)
