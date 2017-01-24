project(darling-libsystem_kernel)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0018 NEW)
endif(COMMAND cmake_policy)

include_directories(${CMAKE_SOURCE_DIR}/kernel-include)
include_directories(${CMAKE_SOURCE_DIR}/src/libc/derived)
include_directories(${CMAKE_SOURCE_DIR}/src/libc/darwin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(emulation/linux)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse2 -msse3 -w -fblocks -ggdb")

# Why -bind_at_load is necessary:
# I couldn't find something like -Bsymbolic in Apple's ld
# -> internal functions are being resolved by dyld
# -> dyld places stubs -> stubs go into dyld_stub_binder at first use
# -> there is a temporary loss of 16-byte stack alignment in __darling_bsd_syscall
# -> dyld_stub_binder checks for this and bails out with a SIGSEGV
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__DARWIN_UNIX03 -fPIC -w -nostdinc -Wl,-bind_at_load -Wl,-undefined,suppress -Wl,-flat_namespace")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")


add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__ -DPRIVATE=1)

add_subdirectory(libsyscall)
add_subdirectory(mach_server/client)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_kernel.dylib")
add_darling_library(system_kernel SHARED
	$<TARGET_OBJECTS:libsyscall>
	$<TARGET_OBJECTS:libsyscall_dynamic>
	#$<TARGET_OBJECTS:kqueue>
	$<TARGET_OBJECTS:emulation>
	${extra_objs}
	$<TARGET_OBJECTS:mach_server_client>)
target_link_libraries(system_kernel system_duct)
make_fat(system_kernel)

add_library(system_kernel_static STATIC
	$<TARGET_OBJECTS:libsyscall>
	$<TARGET_OBJECTS:libsyscall_static>
	#$<TARGET_OBJECTS:kqueue>
	$<TARGET_OBJECTS:emulation_dyld>
	${extra_objs}
	$<TARGET_OBJECTS:mach_server_client_dyld>)

install(TARGETS system_kernel DESTINATION libexec/darling/usr/lib/system)

add_dependencies(system_kernel migcom)
