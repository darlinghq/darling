project(darling-libsystem_kernel)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse2 -msse3 -w -fblocks")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__DARWIN_UNIX03 -fPIC -w -nostdinc")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")
set(MIG_EXECUTABLE "${CMAKE_BINARY_DIR}/mig")

FUNCTION(mig defFileName)
	
	get_directory_property(DirDefs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMPILE_DEFINITIONS)
	get_directory_property(InclDirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} INCLUDE_DIRECTORIES)
	
	set(MIG_FLAGS "")
	foreach (def ${DirDefs})
		set(MIG_FLAGS "${MIG_FLAGS} -D${def}")
	endforeach()
	foreach (dir ${InclDirs})
		set(MIG_FLAGS "${MIG_FLAGS} -I${dir}")
	endforeach()
	
	# message(STATUS "MIG flags: ${MIG_FLAGS}")
	separate_arguments(MIG_FLAGS)
	
	string(REPLACE ".defs" "" bareName "${defFileName}")
	get_filename_component(bareName "${bareName}" NAME)
	add_custom_command(OUTPUT
		${CMAKE_CURRENT_BINARY_DIR}/${bareName}User.c
		${CMAKE_CURRENT_BINARY_DIR}/${bareName}.h
		${CMAKE_CURRENT_BINARY_DIR}/${bareName}Server.c
		${CMAKE_CURRENT_BINARY_DIR}/${bareName}Server.h
		COMMAND
		${MIG_EXECUTABLE}
		-user ${CMAKE_CURRENT_BINARY_DIR}/${bareName}User.c
		-header ${CMAKE_CURRENT_BINARY_DIR}/${bareName}.h
		-server ${CMAKE_CURRENT_BINARY_DIR}/${bareName}Server.c
		-sheader ${CMAKE_CURRENT_BINARY_DIR}/${bareName}Server.h
		${MIG_FLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/${defFileName}
		DEPENDS "${CMAKE_BINARY_DIR}/mig"
	)
	#execute_process(COMMAND ${MIG_EXECUTABLE}
	#	-user ${CMAKE_CURRENT_BINARY_DIR}/${bareName}User.c
	#	-header ${CMAKE_CURRENT_BINARY_DIR}/${bareName}.h
	#	-server ${CMAKE_CURRENT_BINARY_DIR}/${bareName}Server.c
	#	-sheader ${CMAKE_CURRENT_BINARY_DIR}/${bareName}Server.h
	#	${MIG_FLAGS}
	#	${CMAKE_CURRENT_SOURCE_DIR}/${defFileName}
	#	RESULT_VARIABLE MigResult
	#)
	
	#if(NOT "${MigResult}" STREQUAL 0)
	#	message(SEND_ERROR "mig failed")
	#endif()
ENDFUNCTION(mig)

if(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")
	set(BITS 32)
	add_definitions(-DTARGET_CPU_X86=1)
else(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")
	set(BITS 64)
	add_definitions(-DTARGET_CPU_X86_64=1)
endif(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../kernel-include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__)

add_subdirectory(libsyscall)

add_library(system_kernel SHARED
	$<TARGET_OBJECTS:libsyscall>)

add_dependencies(system_kernel migcom)
