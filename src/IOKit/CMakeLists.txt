project(IOKit)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -fconstant-string-class=NSConstantString -fobjc-nonfragile-abi -ggdb")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../util)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/libobjc2)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/corefoundation/Headers)
include_directories(${CMAKE_BINARY_DIR}/src/external/corefoundation/Headers)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/foundation/Headers)

add_definitions(-DOBJC2RUNTIME)

set(IOKit_SRCS
	bsd.cpp
	port.c
	service.mm
	io_object.cpp
	io_iterator.cpp
	io_device_iterator.cpp
	io_device.cpp
	io_device_usb.cpp
	io_device_net.cpp
	../util/debug.cpp
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${SUFFIX}/darling")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(IOKit SHARED ${IOKit_SRCS})
target_link_libraries(IOKit -ludev -lstdc++ CFF)

install(TARGETS IOKit DESTINATION "lib${SUFFIX}/darling")

