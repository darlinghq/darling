project(SystemConfiguration)

set(CMAKE_CXX_STANDARD 11)

set(SystemConfiguration_SRCS
	SCDynamicStoreCopySpecific.cpp
	SCSchemaDefinitions.c
	SCNetworkReachability.c
	SCNetworkConnection.c
	stubs.c
)

add_framework(SystemConfiguration
	FAT
	CURRENT_VERSION
	VERSION "A"
	SOURCES
		${SystemConfiguration_SRCS}
	DEPENDENCIES
		system
		CoreFoundation
)
