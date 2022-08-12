project(SystemConfiguration)

remove_sdk_framework(SystemConfiguration)

set(CMAKE_CXX_STANDARD 11)
include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

set(SystemConfiguration_SRCS
	SCDynamicStoreCopySpecific.cpp
	SCSchemaDefinitions.c
	SCNetworkReachability.c
	SCNetworkConnection.c
	stubs.c
)

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(SystemConfiguration
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/SystemConfiguration"
)

add_framework(SystemConfiguration
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		${SystemConfiguration_SRCS}

	DEPENDENCIES
		system
		CoreFoundation
		cxx
)
