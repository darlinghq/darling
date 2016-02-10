project(CoreGraphics)

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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks")

add_definitions(-D__STDC_LIMIT_MACROS)
add_definitions(-DDARLING_BUILD)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../util)

set(CoreGraphics_SRCS
	CGDirectDisplay.mm
	CGGeometry.cpp
	CGLayer.cpp
	CGContext.cpp
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(CoreGraphics SHARED ${CoreGraphics_SRCS})
target_link_libraries(CoreGraphics -lopal -lgnustep-corebase -lX11 -lXrandr
	-lgnustep-corebase ${QT_LIBRARIES})

install(TARGETS CoreGraphics DESTINATION "${CMAKE_INSTALL_LIBDIR}/darling")

