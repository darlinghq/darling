project(darling-libsystem_init)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -ggdb")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,-bind_at_load")

include_directories(${CMAKE_SOURCE_DIR}/src/libc/fbsdcompat)
include_directories(${CMAKE_SOURCE_DIR}/src/libc/darwin)
include_directories(${CMAKE_SOURCE_DIR}/src/kernel/libsyscall/wrappers)
include_directories(${CMAKE_SOURCE_DIR}/src/external/libpthread/private)
include_directories(${CMAKE_SOURCE_DIR}/src/kernel/libsyscall)
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src/dyld-apple/include)

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1 -DHAVE_SYSTEM_CORESERVICES)
add_definitions(-D__APPLE__ -D__DYNAMIC__)

# Hide warnings
add_definitions(
	-Wno-implicit-function-declaration
	-Wno-incompatible-pointer-types-discards-qualifiers
	-Wno-incompatible-pointer-types
)

set(libsystem_sources
	init.c
	dummy.c
	CompatibilityHacks.c
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(DYLIB_INSTALL_NAME "/usr/lib/libSystem.B.dylib")
add_darling_library(system SHARED ${libsystem_sources})
target_link_libraries(system PRIVATE system_malloc system_c system_kernel keymgr sandbox
	system_m system_info system_notify system_quarantine libdispatch_shared launch compiler_rt
	removefile system_copyfile unwind system_coreservices system_dyld macho CommonCrypto
	system_pthread platform) # objc
set_target_properties(system PROPERTIES OUTPUT_NAME "System.B")
set_property(TARGET system APPEND_STRING PROPERTY LINK_FLAGS
	" -sub_library libsystem_malloc \
	-sub_library libsystem_c \
	-sub_library libsystem_kernel \
	-sub_library libkeymgr \
	-sub_library libsystem_m \
	-sub_library libsystem_info \
	-sub_library libsystem_notify \
	-sub_library libquarantine \
	-sub_library libsystem_blocks \
	-sub_library libsystem_duct \
	-sub_library libsystem_pthread \
	-sub_library libsystem_platform \
	-sub_library libdispatch \
	-sub_library liblaunch \
	-sub_library libremovefile \
	-sub_library libcopyfile \
	-sub_library libunwind \
	-sub_library libdyld \
	-sub_library libcompiler_rt \
	-sub_library libmacho \
	-sub_library libcommonCrypto \
	-sub_library libsystem_sandbox \
	-sub_library libsystem_coreservices \
	-Wl,-dylib_compatibility_version,1238.0.0")

make_fat(system)

install(TARGETS system DESTINATION libexec/darling/usr/lib)
InstallSymlink("libSystem.B.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libSystem.dylib")

