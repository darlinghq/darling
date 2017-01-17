project(libmacho)

cmake_minimum_required(VERSION 2.4.0)

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__)
add_definitions(-D__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__=1080)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,-bind_at_load -Wl,-flat_namespace -Wl,-undefined,suppress")

SET(libmacho_sources
	arch.c
	get_end.c
	getsecbyname.c
	getsegbyname.c
	#hppa_swap.c
	i386_swap.c
	#i860_swap.c
	#m68k_swap.c
	#m88k_swap.c
	#ppc_swap.c
	slot_name.c
	#sparc_swap.c
	swap.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libmacho.dylib")
add_darling_library(macho SHARED ${libmacho_sources})
target_link_libraries(macho system_kernel)

install(TARGETS macho DESTINATION libexec/darling/usr/lib/system)
