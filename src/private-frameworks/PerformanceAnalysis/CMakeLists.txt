project(PerformanceAnalysis)

remove_sdk_framework(PerformanceAnalysis
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "148.0.0")

file(GLOB PA_C src/*.c)
file(GLOB PA_OBJC src/*.m)

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(PerformanceAnalysis
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/PerformanceAnalysis"
	PRIVATE
)

add_framework(PerformanceAnalysis
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		${PA_C}
		${PA_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
