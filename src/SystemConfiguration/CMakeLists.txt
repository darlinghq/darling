project(SystemConfiguration)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")

set(SystemConfiguration_SRCS
	SCDynamicStoreCopySpecific.cpp
	SCSchemaDefinitions.c
	SCNetworkReachability.c
	SCNetworkConnection.c
)

add_framework(SystemConfiguration
	FAT
	CURRENT_VERSION
	VERSION "A"
	SOURCES
		${SystemConfiguration_SRCS}
	DEPENDENCIES
		system
		${CoreFoundation}
)
