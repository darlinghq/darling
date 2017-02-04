project(cff)

cmake_minimum_required(VERSION 2.4.0)

enable_language(C ASM)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_definitions(-D__APPLE__ -D__MACH__)
add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__)
add_definitions(-D__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__=1080)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w -fconstant-string-class=NSConstantString -fobjc-nonfragile-abi -ggdb -O0")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -nostdinc -nostdinc++ -D__DARWIN_UNIX03 -fPIC -w -fconstant-string-class=NSConstantString -fobjc-nonfragile-abi -ggdb -O0")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map")
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include_directories(${DARLING_TOP_DIRECTORY}/src/external/corefoundation/Headers)
include_directories(${CMAKE_BINARY_DIR}/src/external/corefoundation/Headers)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/foundation/Headers)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/libobjc2)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/libcxx/include)
include_directories(${DARLING_TOP_DIRECTORY}/src/libc/include/FreeBSD)

add_definitions(-DOBJC2RUNTIME=1)

set(cff_SRCS
	#AppleCFString.m
	$<TARGET_OBJECTS:Foundation>
	$<TARGET_OBJECTS:CoreFoundation>
	#AppleCFString.m
)

set(cffextra_SRCS
	CFUtilities.c
	reexports.c
	#CFPreferences.mm
	../util/debug.cpp
	../util/stlutils.cpp
	../external/libcxx/src/dso_handle.c
)

add_library(CFF SHARED ${cff_SRCS})
set_source_files_properties(${cff_SRCS} LANGUAGE C)
target_link_libraries(CFF PRIVATE system icucore z ffi xml2 xslt iconv aprutil "-Wl,--defsym=__CFConstantStringClassReference=_OBJC_CLASS_AppleCFString")

add_library(CFFExtra SHARED ${cffextra_SRCS})
target_link_libraries(CFFExtra PRIVATE CFF cxx)


install(TARGETS CFF CFFExtra DESTINATION ${CMAKE_INSTALL_LIBDIR}/darling)
