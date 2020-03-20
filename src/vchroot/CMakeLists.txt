project(vchroot)
include(darling_exe)

add_definitions(-nostdinc)

add_darling_executable(vchroot vchroot.c)
target_link_libraries(vchroot system)
install(TARGETS vchroot DESTINATION libexec/darling/usr/libexec/darling)

