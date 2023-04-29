project(CoreData_stub)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")

add_framework(CoreData
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}
	TARGET_NAME CoreData${STUB_SUFFIX}
	${NO_INSTALL_ARG}

	SOURCES
		src/classes.m
		src/main.m

	DEPENDENCIES
		system
		Foundation
)

#target_include_directories(CoreData${STUB_SUFFIX} BEFORE PRIVATE include)
