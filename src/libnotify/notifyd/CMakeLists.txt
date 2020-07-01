project(notifyd)

mig(notify_ipc.defs)
mig(notify.defs)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(notifyd_sources
	notifyd.c
	notify_proc.c
	pathwatch.c
	service.c
	timer.c
	${CMAKE_CURRENT_BINARY_DIR}/notify_ipcServer.c
	${CMAKE_CURRENT_BINARY_DIR}/notifyServer.c
)

add_darling_executable(notifyd ${notifyd_sources})
target_link_libraries(notifyd system bsm.0)

install(TARGETS notifyd DESTINATION libexec/darling/usr/sbin)
install(FILES com.apple.notifyd.plist DESTINATION libexec/darling/System/Library/LaunchDaemons)
install(FILES notifyd.8 DESTINATION libexec/darling/usr/share/man/man8)
install(FILES notify.conf.MacOSX RENAME notify.conf DESTINATION libexec/darling/etc)

