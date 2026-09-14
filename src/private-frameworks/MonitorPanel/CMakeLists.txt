project(MonitorPanel)

remove_sdk_framework(MonitorPanel PRIVATE)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(MonitorPanel
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/MonitorPanel"
	PRIVATE
)

add_framework(MonitorPanel
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/MPDisplay.m

	DEPENDENCIES
		system
		objc
		Foundation
)
