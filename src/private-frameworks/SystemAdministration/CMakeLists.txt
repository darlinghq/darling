project(SystemAdministration)

remove_sdk_framework(SystemAdministration
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(SystemAdministration
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/SystemAdministration"
	PRIVATE
)

add_framework(SystemAdministration
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/SystemAdministration.m

	DEPENDENCIES
		system
		objc
		Foundation
)
