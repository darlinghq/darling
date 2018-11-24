project(tools)

add_darling_executable(sw_vers sw_vers.c)
add_darling_executable(sudo sudo.c)
add_darling_executable(codesign codesign.c)

install(TARGETS sw_vers sudo codesign DESTINATION libexec/darling/usr/bin)
target_link_libraries(sw_vers CoreFoundation)
