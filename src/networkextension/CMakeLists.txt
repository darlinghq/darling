project(system_networkextension)

cmake_minimum_required(VERSION 2.4.0)

include_directories(${CMAKE_SOURCE_DIR}/src/libc/darwin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_networkextension.dylib")
add_darling_library(system_networkextension
    SHARED
        ne_session.c
        NEHelperClient.c
)
target_link_libraries(system_networkextension
    PRIVATE
        system_c
        system_dyld
)
make_fat(system_networkextension)

install(TARGETS system_networkextension DESTINATION libexec/darling/usr/lib/system)
