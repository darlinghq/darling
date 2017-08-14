project(kperf)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(kperf
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		src/kperf.c

	DEPENDENCIES
		system
)
