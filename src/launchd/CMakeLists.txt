project(launchd-top)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_subdirectory(liblaunch)
add_subdirectory(src)
add_subdirectory(support)

install(FILES man/launchctl.1 DESTINATION libexec/darling/usr/share/man/man1)
install(FILES man/launchd.8 DESTINATION libexec/darling/usr/share/man/man8)

install(DIRECTORY DESTINATION libexec/darling/Library/LaunchDaemons)
install(DIRECTORY DESTINATION libexec/darling/Library/LaunchAgents)
