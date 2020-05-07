project(SystemConfiguration)

set(CMAKE_CXX_STANDARD 11)
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src/external/libcxx/include ${CMAKE_CURRENT_BINARY_DIR})

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
		cxx
)
