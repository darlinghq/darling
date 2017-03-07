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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(ffi_SRCS
	ffi.c
	types.c
	x86/darwin64.S
	x86/x86-darwin.S
	x86/x86-ffi64.c
	x86/x86-ffi_darwin.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/libffi.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(ffi SHARED ${ffi_SRCS})
target_link_libraries(ffi system)
make_fat(ffi)

install(TARGETS ffi DESTINATION libexec/darling/usr/lib)
