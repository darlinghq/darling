project(DiskImages)

remove_sdk_framework(DiskImages
	PRIVATE
)

add_compile_options(
	-nostdinc
)

set(DYLIB_COMPAT_VERSION "1.0.8")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(DiskImages
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/DiskImages"
	PRIVATE
)

add_framework(DiskImages
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}
	CURRENT_VERSION
	SOURCES
		src/functions.c
		#src/DIHelperProxy.m
		src/NSDictionary+BetterAccessors.m
	DEPENDENCIES
		system
		Foundation
)
