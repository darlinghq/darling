project(tools)

add_darling_executable(sw_vers sw_vers.c)
add_darling_executable(sudo sudo.c)
add_darling_executable(codesign codesign.c)
add_darling_executable(open open.m)
add_darling_executable(dsmemberutil dsmemberutil.c)
add_darling_executable(softwareupdate softwareupdate.c)

install(TARGETS sw_vers sudo codesign dsmemberutil open DESTINATION libexec/darling/usr/bin)
install(TARGETS softwareupdate DESTINATION libexec/darling/usr/sbin)

target_link_libraries(sw_vers CoreFoundation)
target_link_libraries(open CoreServices Foundation AppKit)
