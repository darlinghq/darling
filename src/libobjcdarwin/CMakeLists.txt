project(objc.A.dylib)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

enable_language(ASM_NASM)

#if (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")
#	message(FATAL_ERROR "Clang is the only supported compiler.")
#endif (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")

#configure_file(config.h.in config.h)

add_definitions(-D__STDC_LIMIT_MACROS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -fPIC -fvisibility=hidden -ggdb")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -fPIC -fvisibility=hidden -ggdb")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC")

set(objcdarwin_SRCS
	objc_msgSend_fixup.nasm
	objc_msgSendSuper.nasm
	objc_msgSend.nasm
	NameTranslate.cpp
	ClassRegister.cpp
	TrampolineHelper.mm
	
	common/attribute.cpp
	common/property.cpp
	common/selector.cpp
	
	old/category.cpp
	old/class.cpp
	old/ivar.mm
	old/protocol.cpp
	
	new/category.cpp
	new/class.cpp
	new/ivar.cpp
	new/protocol.cpp
	
	NSBundle_dyld.mm
	NSCFString.mm
	misc.mm
	zero.c
)

add_library(objc.A.dylib SHARED ${objcdarwin_SRCS})
target_link_libraries(objc.A.dylib -l:libobjc.so.4 -lgnustep-base -l:../../libutil.a)

install(TARGETS objc.A.dylib DESTINATION "lib${SUFFIX}")

