project(CrashReporterSupport)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "827.0.0")

file(GLOB CRS_C src/*.c)
file(GLOB CRS_OBJC src/*.m)

add_framework(CrashReporterSupport
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		${CRS_C}
		${CRS_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
