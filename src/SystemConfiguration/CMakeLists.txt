project(SystemConfiguration)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


#configure_file(config.h.in config.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/corefoundation/Headers)
include_directories(${CMAKE_BINARY_DIR}/src/external/corefoundation/Headers)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/foundation/Headers)
include_directories(${DARLING_TOP_DIRECTORY}/basic-headers)

set(SystemConfiguration_SRCS
	SCDynamicStoreCopySpecific.cpp
	SCSchemaDefinitions.cpp
)

add_library(SystemConfiguration SHARED ${SystemConfiguration_SRCS})
target_link_libraries(SystemConfiguration CFF)

install(TARGETS SystemConfiguration DESTINATION "${CMAKE_INSTALL_LIBDIR}/darling")

