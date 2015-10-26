project(libobjc)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

enable_language(ASM_NASM)

#if (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")
#	message(FATAL_ERROR "Clang is the only supported compiler.")
#endif (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")

if (BITS EQUAL 64) # Would need to be extended for ARM
	set(OBJC_ABI_2 TRUE)
	add_definitions(-DOBJC_ABI_2=1)
	message(STATUS "Building ObjC ABI 2")
endif(BITS EQUAL 64)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-D__STDC_LIMIT_MACROS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -fPIC -fvisibility=hidden -fconstant-string-class=NSConstantString -ggdb")
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
	common/cfstring.cpp
	
	NSDarwinFramework.mm
	misc.mm
)

if(NOT OBJC_ABI_2)
	set(objcdarwin_SRCS
		${objcdarwin_SRCS}
		
		old/exceptions.cpp
		old/category.cpp
		old/class.cpp
		old/ivar.mm
		old/protocol.cpp
	)
else(NOT OBJC_ABI_2)
	set(objcdarwin_SRCS
		${objcdarwin_SRCS}
		
		new/exceptions.cpp
		new/category.cpp
		new/class.cpp
		new/ivar.cpp
		new/protocol.cpp
		
		new/return.nasm
	)
endif(NOT OBJC_ABI_2)

add_library(objcdarwin SHARED ${objcdarwin_SRCS})
target_link_libraries(objcdarwin Foundation util ${GNUSTEP_OBJC_CFLAGS})

install(TARGETS objcdarwin DESTINATION "${CMAKE_INSTALL_LIBDIR}/darling")

