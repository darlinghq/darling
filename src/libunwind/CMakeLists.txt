project(darling-libsystem_unwind)

cmake_minimum_required(VERSION 2.4.0)
enable_language(C ASM)

set(CMAKE_CXX_LINKER_PREFERENCE_PROPAGATES 0)
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdinc -nostdinc++ -fno-rtti -fno-exceptions -funwind-tables -ggdb -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse2 -msse3 -w -nostdinc -fno-rtti -funwind-tables -ggdb -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__DARWIN_UNIX03 -fPIC -w")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,-flat_namespace -Wl,-undefined,suppress")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/../external/libcxx/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libc/locale)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libc/locale/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libc/stdtime/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libc/pthreads)
include_directories(${CMAKE_SOURCE_DIR}/platform-include)
include_directories(${CMAKE_SOURCE_DIR}/src/libc/darwin)
include_directories(${CMAKE_SOURCE_DIR}/src/external/libpthread/private)
include_directories(${CMAKE_SOURCE_DIR}/src/kernel/libsyscall/wrappers)
include_directories(${CMAKE_SOURCE_DIR}/src/kernel/libsyscall)

set(libunwind_sources
	src/libuwind.cxx
	src/Registers.S
	src/unw_getcontext.S
	src/UnwindLevel1.c
	src/UnwindLevel1-gcc-ext.c
	src/Unwind-sjlj.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libunwind.dylib")
add_circular(unwind FAT
	SOURCES
		${libunwind_sources}
	SIBLINGS
		system_c
		keymgr
)
#target_link_libraries(unwind PRIVATE system_c keymgr)
#set_target_properties(unwind PROPERTIES LINKER_LANGUAGE C)
#make_fat(unwind)

include(darling_static_lib)
add_darling_static_library(unwind_static SOURCES ${libunwind_sources})
set_target_properties(unwind_static PROPERTIES COMPILE_FLAGS "-DNDEBUG=1")
make_fat(unwind_static)

install(TARGETS unwind DESTINATION libexec/darling/usr/lib/system)
