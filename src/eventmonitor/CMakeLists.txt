project(eventmonitor)

add_definitions(-nostdinc)
add_compile_options(-fblocks)

add_darling_executable(eventmonitor eventmonitor.c)
target_link_libraries(eventmonitor CoreFoundation CoreServices)

install(TARGETS eventmonitor DESTINATION libexec/darling/usr/libexec)
install(FILES org.darlinghq.eventmonitor.plist DESTINATION libexec/darling/System/Library/LaunchDaemons)
