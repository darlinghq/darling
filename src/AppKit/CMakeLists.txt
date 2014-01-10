project(AppKit)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(QT_USE_QTDECLARATIVE TRUE)
find_package(Qt4 REQUIRED)

add_definitions(${QT_DEFINITIONS})
include_directories(${QT_INCLUDE_DIR})
include(${QT_USE_FILE})

#if (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")
#	message(FATAL_ERROR "Clang is the only supported compiler.")
#endif (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")

#configure_file(config.h.in config.h)
add_definitions(-D__STDC_LIMIT_MACROS)
add_definitions(-DDARLING_BUILD)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fconstant-string-class=NSConstantString")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../util)

set(AppKit_SRCS
	NSApplication.mm
	NSEvent.mm
	NSX11Event.mm
	QNSEventDispatcher.mm
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${SUFFIX}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(AppKit SHARED ${AppKit_SRCS})
target_link_libraries(AppKit -lgnustep-base ${QT_LIBRARIES})

install(TARGETS AppKit DESTINATION "lib${SUFFIX}/darling")

