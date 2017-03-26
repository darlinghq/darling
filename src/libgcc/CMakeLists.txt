project(libgcc)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,-sub_library,libcompiler_rt")

set(libgcc_sources
	dummy.c
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(DYLIB_INSTALL_NAME "/usr/lib/libgcc_s.1.dylib")
add_darling_library(gcc_s SHARED ${libgcc_sources})
make_fat(gcc_s)
target_link_libraries(gcc_s PRIVATE compiler_rt)
set_target_properties(gcc_s PROPERTIES OUTPUT_NAME "gcc_s.1")

InstallSymlink("libgcc_s.1.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libgcc_s.dylib")

install(TARGETS gcc_s DESTINATION libexec/darling/usr/lib)
