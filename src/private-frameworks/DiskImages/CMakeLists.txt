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
		src/functions.c
		#src/DIHelperProxy.m
		src/NSDictionary+BetterAccessors.m
	DEPENDENCIES
		system
		Foundation
)
