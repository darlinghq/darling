project(DiskImages)

add_compile_options(
	-nostdinc
)

set(DYLIB_COMPAT_VERSION "1.0.8")

add_framework(DiskImages
	PRIVATE
	VERSION "A"
	CURRENT_VERSION
	SOURCES
		functions.c
		#DIHelperProxy.m
	DEPENDENCIES
		system
		Foundation
)
