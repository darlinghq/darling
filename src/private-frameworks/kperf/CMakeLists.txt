project(kperf)

remove_sdk_framework(kperf
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(kperf
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/kperf"
	PRIVATE
)

add_framework(kperf
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/kperf.c

	DEPENDENCIES
		system
)
