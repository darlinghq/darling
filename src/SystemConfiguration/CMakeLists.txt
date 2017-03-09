project(SystemConfiguration)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


#configure_file(config.h.in config.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_SOURCE_DIR}/src/external/corefoundation/)
include_directories(${CMAKE_SOURCE_DIR}/src/external/foundation/include)
include_directories(${CMAKE_SOURCE_DIR}/src/external/libclosure)
include_directories(${CMAKE_SOURCE_DIR}/src/launchd/liblaunch)

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
		CoreFoundation
)


