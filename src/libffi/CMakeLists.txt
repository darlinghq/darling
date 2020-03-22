project(libffi)

cmake_minimum_required(VERSION 2.4.0)

enable_language(C ASM)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_definitions(-D__APPLE__ -D__MACH__)
add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__)
#add_definitions(-D__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__=1080)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

include_directories(${CMAKE_SOURCE_DIR}/platform-include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/darwin/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(ffi_SRCS
	src/closures.c
	# src/debug.c
	# src/dlmalloc.c
	src/java_raw_api.c
	src/prep_cif.c
	src/raw_api.c
	src/types.c

	src/x86/sysv.S
	src/x86/ffi.c
	src/x86/ffi64.c
	src/x86/unix64.S
	src/x86/ffiw64.c
	src/x86/win64.S

	src/arm/sysv.S
	src/arm/ffi.c

	src/aarch64/sysv.S
	src/aarch64/ffi.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/libffi.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(ffi SHARED ${ffi_SRCS})
target_link_libraries(ffi system)
make_fat(ffi)

install(TARGETS ffi DESTINATION libexec/darling/usr/lib)
