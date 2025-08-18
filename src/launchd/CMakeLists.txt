project(launchd-top)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_subdirectory(liblaunch)
add_subdirectory(src)
add_subdirectory(support)

install(FILES man/launchctl.1 DESTINATION libexec/darling/usr/share/man/man1)
install(FILES man/launchd.8 DESTINATION libexec/darling/usr/share/man/man8)

# From xcscripts/launchctl-postflight.sh
install(DIRECTORY DESTINATION libexec/darling/System/Library/LaunchAgents)
install(DIRECTORY DESTINATION libexec/darling/System/Library/LaunchDaemons)
install(DIRECTORY DESTINATION libexec/darling/Library/LaunchAgents)
install(DIRECTORY DESTINATION libexec/darling/Library/LaunchDaemons)

# From xcscripts/launchd-postflight.sh
install(DIRECTORY DESTINATION libexec/darling/private/var/db/launchd.db/com.apple.launchd)
install(DIRECTORY DESTINATION libexec/darling/private/var/log/com.apple.launchd)
install(DIRECTORY DESTINATION libexec/darling/private/etc/mach_init.d)
install(DIRECTORY DESTINATION libexec/darling/private/etc/mach_init_per_user.d)
install(DIRECTORY DESTINATION libexec/darling/private/etc/mach_init_per_login_session.d)

# From xcscripts/SystemStarter-postflight.sh
install(DIRECTORY DESTINATION libexec/darling/System/Library/StartupItems)
install(DIRECTORY DESTINATION libexec/darling/Library/StartupItems)
