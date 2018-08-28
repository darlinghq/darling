project(CoreServices)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions -nostdinc -nostdinc++")

include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src/external/libcxx/include)
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/src/libc/include/FreeBSD
	${CMAKE_SOURCE_DIR}/src/libiconv/include
	${CMAKE_SOURCE_DIR}/src/external/icu/icuSources/common
	${CMAKE_SOURCE_DIR}/src/external/icu/icuSources/i18n
	${CMAKE_SOURCE_DIR}/src/external/corefoundation
	${CMAKE_SOURCE_DIR}/src/external/foundation/include
	${CMAKE_SOURCE_DIR}/src/external/libclosure
)

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
	CoreServicesPriv.cpp
	TextCommon.cpp
	TextEncodingConverter.cpp
	MDQuery.c
	LSApplicationProxy.m
	LSApplicationWorkspace.m
)

if (WITH_COREAUDIO)
	set(EXTRA_LIBS AudioUnit)
endif (WITH_COREAUDIO)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
add_framework(CoreServices
	FAT
	CURRENT_VERSION
	VERSION "A"
	SOURCES
		${CoreServices_SRCS}
	DEPENDENCIES
		icucore
		system
		CoreFoundation
		cxx
		iconv
		Foundation
		${EXTRA_LIBS}
)

install(FILES SystemVersion.plist DESTINATION "libexec/darling/System/Library/CoreServices")

