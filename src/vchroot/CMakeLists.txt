project(vchroot)
include(darling_exe)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")

add_darling_executable(vchroot vchroot.c)
target_link_libraries(vchroot system)
install(TARGETS vchroot DESTINATION libexec/darling/usr/libexec/darling)

