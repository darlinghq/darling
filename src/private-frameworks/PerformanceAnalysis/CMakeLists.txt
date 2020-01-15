project(PerformanceAnalysis)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "148.0.0")

file(GLOB PA_C src/*.c)
file(GLOB PA_OBJC src/*.m)

add_framework(PerformanceAnalysis
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		${PA_C}
		${PA_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
