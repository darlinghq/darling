project(ServerInformation)

remove_sdk_framework(ServerInformation
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ServerInformation
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ServerInformation"
	PRIVATE
)

add_framework(ServerInformation
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/ServerInformation.m
		src/ServerInformationComputerModelInfo.m

	DEPENDENCIES
		system
		objc
		Foundation
)
