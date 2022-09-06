project(CrashReporterSupport)

remove_sdk_framework(CrashReporterSupport
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "827.0.0")

file(GLOB CRS_C src/*.c)
file(GLOB CRS_OBJC src/*.m)

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CrashReporterSupport
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CrashReporterSupport"
	PRIVATE
)

add_framework(CrashReporterSupport
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		${CRS_C}
		${CRS_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
