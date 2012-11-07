project(runtest)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

#if (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")
#	message(FATAL_ERROR "Clang is the only supported compiler.")
#endif (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -fPIC")

set(runtest_SRCS
	runtest.cpp
	termcolor.cpp
	pstream.cpp
	boostxml.cpp
	timer.cpp
)

add_executable(runtest ${runtest_SRCS})
target_link_libraries(runtest sshcxx)

add_subdirectory(libsshcxx)
add_dependencies(runtest sshcxx)

#install(TARGETS runtest DESTINATION "lib${SUFFIX}/darling")

