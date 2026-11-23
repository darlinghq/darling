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
		src/ctkatr.c
		src/ctkclient_test.c
		src/ctkcodec.c
		src/ctkloginhelper.c
		src/TKSmartCardATR.m
		src/TKTLVRecord.m
		src/TKTokenWatcher.m

	DEPENDENCIES
		system
		CoreFoundation
		Foundation
)
