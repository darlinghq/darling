project(darling-libsystem_kernel)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0018 NEW)
endif(COMMAND cmake_policy)

add_subdirectory(emulation/linux)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse2 -msse3 -w -fblocks -ggdb")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__DARWIN_UNIX03 -fPIC -w -nostdinc -Wl,-undefined,suppress -Wl,-flat_namespace")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

include_directories(${DARLING_TOP_DIRECTORY}/kernel-include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__ -DPRIVATE)

add_subdirectory(libsyscall)
add_subdirectory(mach_server/client)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_kernel.dylib")
add_darling_library(system_kernel SHARED
	$<TARGET_OBJECTS:libsyscall>
	#$<TARGET_OBJECTS:kqueue>
	$<TARGET_OBJECTS:emulation>
	${extra_objs}
	$<TARGET_OBJECTS:mach_server_client>)
target_link_libraries(system_kernel system_duct)

install(TARGETS system_kernel DESTINATION ${CMAKE_INSTALL_LIBDIR}/darling)

add_dependencies(system_kernel migcom)
