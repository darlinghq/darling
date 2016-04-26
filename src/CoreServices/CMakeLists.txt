project(CoreServices)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

#if (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")
#	message(FATAL_ERROR "Clang is the only supported compiler.")
#endif (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")

#configure_file(config.h.in config.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions -nostdinc -nostdinc++ -ggdb -O0")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map -nostdlib -Bsymbolic")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${DARLING_TOP_DIRECTORY}/platform-include)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/libcxx/include)
include_directories(${DARLING_TOP_DIRECTORY}/src/libc/include/FreeBSD)
include_directories(${DARLING_TOP_DIRECTORY}/src/libiconv/include)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/icu/icuSources/common)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/icu/icuSources/i18n)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/corefoundation/Headers)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/foundation/Headers)
include_directories(${CMAKE_BINARY_DIR}/src/external/corefoundation/Headers)
include_directories(${CMAKE_BINARY_DIR})
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../util)

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
	LaunchServices.cpp
	OpenTransport.cpp
	Timer.cpp
	../external/libcxx/src/dso_handle.c
)

if (WITH_COREAUDIO)
	set(EXTRA_LIBS AudioUnit)
endif (WITH_COREAUDIO)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(CoreServices SHARED ${CoreServices_SRCS})
target_link_libraries(CoreServices icucore system ${EXTRA_LIBS} CFF cxx iconv)

install(TARGETS CoreServices DESTINATION "${CMAKE_INSTALL_LIBDIR}/darling")
install(FILES SystemVersion.plist DESTINATION "libexec/darling/System/Library/CoreServices")

