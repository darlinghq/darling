project(launchd-top)

add_subdirectory(liblaunch)
add_subdirectory(src)
add_subdirectory(support)

install(FILES man/launchctl.1 DESTINATION libexec/darling/usr/share/man/man1)
install(FILES man/launchd.8 DESTINATION libexec/darling/usr/share/man/man8)
