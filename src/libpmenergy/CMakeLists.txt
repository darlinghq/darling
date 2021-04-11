project(pmenergy)

cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w -ggdb")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

set(DYLIB_INSTALL_NAME "/usr/lib/libpmenergy.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(pmenergy SHARED src/pmenergy.c)
target_link_libraries(pmenergy PRIVATE system)

###

set(DYLIB_INSTALL_NAME "/usr/lib/libpmsample.dylib")
add_darling_library(pmsample SHARED src/pmsample.c)
target_link_libraries(pmsample PRIVATE system)

install(TARGETS pmenergy pmsample DESTINATION libexec/darling/usr/lib)

