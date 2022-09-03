project(AuthKit)

remove_sdk_framework(AuthKit
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

file(GLOB AK_C src/*.c)
file(GLOB AK_OBJC src/*.m)

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AuthKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AuthKit"
	PRIVATE
)

add_framework(AuthKit
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		${AK_C}
		${AK_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
