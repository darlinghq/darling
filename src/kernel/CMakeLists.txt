project(darling-libsystem_kernel)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0018 NEW)
endif(COMMAND cmake_policy)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(emulation/linux)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse2 -msse3 -w -fblocks -ffreestanding")

# Why -bind_at_load is necessary:
# I couldn't find something like -Bsymbolic in Apple's ld
# -> internal functions are being resolved by dyld
# -> dyld places stubs -> stubs go into dyld_stub_binder at first use
# -> there is a temporary loss of 16-byte stack alignment in __darling_bsd_syscall
# -> dyld_stub_binder checks for this and bails out with a SIGSEGV
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__DARWIN_UNIX03 -fPIC -w -nostdinc -Wl,-bind_at_load")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

#set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_c.dylib")
#add_darling_library(fakelibc SHARED fakelibc.c)
#make_fat(fakelibc)
#set(DYLIB_INSTALL_NAME "/usr/lib/system/libdyld.dylib")
#add_darling_library(fakedyld SHARED fakedyld.c)
#make_fat(fakedyld)

add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-DPRIVATE=1)

add_subdirectory(libsyscall)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_kernel.dylib")
add_circular(system_kernel FAT
	OBJECTS
		$<IF:$<BOOL:${BUILD_TARGET_32BIT}>,$<TARGET_OBJECTS:libsyscall_32>,>
		$<IF:$<BOOL:${BUILD_TARGET_64BIT}>,$<TARGET_OBJECTS:libsyscall_64>,>
		$<TARGET_OBJECTS:libsyscall>
		$<TARGET_OBJECTS:libsyscall_dynamic>
		#$<TARGET_OBJECTS:kqueue>
		$<TARGET_OBJECTS:emulation>
		$<TARGET_OBJECTS:mach_server_client>
	SIBLINGS
		system_c
		compiler_rt
		system_dyld
)
#target_link_libraries(system_kernel system_duct platform_static32 platform_static64)
make_fat(system_kernel)

add_library(system_kernel_static32 STATIC
	$<TARGET_OBJECTS:libsyscall_32>
	$<TARGET_OBJECTS:libsyscall>
	$<TARGET_OBJECTS:emulation_dyld>
	$<TARGET_OBJECTS:mach_server_client_dyld>
)

add_library(system_kernel_static64 STATIC
	$<TARGET_OBJECTS:libsyscall_64>
	$<TARGET_OBJECTS:libsyscall>
	$<TARGET_OBJECTS:emulation_dyld>
	$<TARGET_OBJECTS:mach_server_client_dyld>
)

install(TARGETS system_kernel DESTINATION libexec/darling/usr/lib/system)

add_dependencies(system_kernel migcom)
