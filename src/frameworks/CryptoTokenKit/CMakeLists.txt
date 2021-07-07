project(CryptoTokenKit)

add_compile_options(
	-nostdinc
	-fblocks
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(CryptoTokenKit
	FAT
	CURRENT_VERSION
	VERSION "A"

	SOURCES
		src/ctkclient.c
		src/ctkclient_test.c
		src/ctkloginhelper.c

	DEPENDENCIES
		system
		CoreFoundation
)
