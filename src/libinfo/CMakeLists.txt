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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w -ggdb")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

add_subdirectory(gen.subproj)
add_subdirectory(util.subproj)
add_subdirectory(dns.subproj)
add_subdirectory(nis.subproj)
add_subdirectory(rpc.subproj)
add_subdirectory(lookup.subproj)
add_subdirectory(membership.subproj)
add_subdirectory(darling-resolver)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_info.dylib")
add_darling_library(system_info SHARED $<TARGET_OBJECTS:info-util>
	$<TARGET_OBJECTS:info-dns>
	$<TARGET_OBJECTS:info-gen>
	$<TARGET_OBJECTS:info-nis>
	$<TARGET_OBJECTS:info-rpc>
	$<TARGET_OBJECTS:info-lookup>
	$<TARGET_OBJECTS:info-membership>
	$<TARGET_OBJECTS:darling-resolver>
)
target_link_libraries(system_info PRIVATE system_c resolv-darwin system_dyld system_blocks libdispatch_shared system_notify)

install(TARGETS system_info DESTINATION libexec/darling/usr/lib/system)
install(FILES master.passwd DESTINATION libexec/darling/etc)

