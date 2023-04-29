project(Cocoa_stub)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")

add_framework(Cocoa
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}
	TARGET_NAME Cocoa${STUB_SUFFIX}
	${NO_INSTALL_ARG}

	SOURCES
		src/classes.m
		src/main.m

	DEPENDENCIES
		system
		Foundation
)

#target_include_directories(Cocoa${STUB_SUFFIX} BEFORE PRIVATE include)
