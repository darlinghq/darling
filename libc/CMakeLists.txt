project(darling-libsystem_c)

cmake_minimum_required(VERSION 2.4.0)

# -DHAVE_STDINT_H=1 -I libc/include -I platform-include/ -I kernel-include/ -I
# libc/locale/ -I libc/locale/FreeBSD/ -I libc/stdtime/FreeBSD -I
# platform-include/pthread/ -I libc/fbsdcompat -I libc/pthreads -nostdinc
# -DTARGET_CPU_X86_64=1 -DTARGET_OS_MAC=1

if(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")
	set(BITS 32)
	add_definitions(-DTARGET_CPU_X86=1)
else(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")
	set(BITS 64)
	add_definitions(-DTARGET_CPU_X86_64=1)
endif(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions("'-D__weak_reference(sym,alias)=__asm__(\".weak alias\");__asm__(\".equ alias, sym\")'")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/pthreads")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/darwin")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../libmalloc/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/locale")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/gen")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/locale/FreeBSD")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/NetBSD")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/stdtime/FreeBSD")

add_subdirectory(db) # produces libc-db
add_subdirectory(compat-43) # produces libc-compat
add_subdirectory(emulated) # produces libc-emulated
add_subdirectory(gdtoa) # produces libc-gdtoa
add_subdirectory(gen)

