project(libinfo)

cmake_minimum_required(VERSION 2.4.0)

if(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")
	set(BITS 32)
	add_definitions(-DTARGET_CPU_X86=1)
else(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")
	set(BITS 64)
	add_definitions(-DTARGET_CPU_X86_64=1)
endif(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__)
add_definitions(-D__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__=1080)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,--unresolved-symbols=ignore-all")

add_subdirectory(util.subproj)
add_subdirectory(dns.subproj)
#add_subdirectory(lookup.subproj)

add_library(system_info SHARED $<TARGET_OBJECTS:info-util>
	$<TARGET_OBJECTS:info-dns>
	#$<TARGET_OBJECTS:info-lookup> # lookup doesn't build yet
)
target_link_libraries(system_info system_c)
