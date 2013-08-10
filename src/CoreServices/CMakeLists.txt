project(CoreServices)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

#if (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")
#	message(FATAL_ERROR "Clang is the only supported compiler.")
#endif (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")

#configure_file(config.h.in config.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../util)

set(CoreServices_SRCS
	CoreEndian.cpp
	Multiprocessing.cpp
	TextUtils.cpp
	MacMemory.cpp
	MacLocales.cpp
	UnicodeUtilities.cpp
	Gestalt.cpp
	FileManager.cpp
	DateTimeUtils.cpp
	FixMath.cpp
	ToolUtils.cpp
	Math64.cpp
	DriverSynchronization.cpp
	DriverServices.cpp
	Components.cpp
	Processes.cpp
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${SUFFIX}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(CoreServices SHARED ${CoreServices_SRCS})
target_link_libraries(CoreServices -licuuc System.B.dylib util AudioUnit
	-lgnustep-corebase)

install(TARGETS CoreServices DESTINATION "lib${SUFFIX}/darling")

