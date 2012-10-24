project(CoreServices)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

enable_language(ASM_NASM)

#if (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")
#	message(FATAL_ERROR "Clang is the only supported compiler.")
#endif (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")

#configure_file(config.h.in config.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks")

#find_path(LIBCXX_INCLUDE_DIR c++/v1/vector)
#if (DEFINED LIBCXX_INCLUDE_DIR)
#    message(STATUS "Found libcxx in ${LIBCXX_INCLUDE_DIR}")
#else (DEFINED LIBCXX_INCLUDE_DIR)
#    message(FATAL_ERROR "libcxx not found")
#endif (DEFINED LIBCXX_INCLUDE_DIR)

#include_directories(${LIBCXX_INCLUDE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../util)

set(CoreServices_SRCS
	MacLocales.cpp
	UnicodeUtilities.cpp
	Gestalt.cpp
)

add_library(CoreServices SHARED ${CoreServices_SRCS})
target_link_libraries(CoreServices -licuuc)

install(TARGETS CoreServices DESTINATION "lib${SUFFIX}/darling")

