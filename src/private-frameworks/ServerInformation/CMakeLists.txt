project(ServerInformation)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(ServerInformation
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		src/ServerInformation.m
		src/ServerInformationComputerModelInfo.m

	DEPENDENCIES
		system
		objc
		Foundation
)
