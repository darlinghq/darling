set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1239.200.12")
add_framework(FSEvents
	FAT
	CURRENT_VERSION
	VERSION "A"
	PARENT "CoreServices"
	SOURCES
		FSEvents.m
		FSEventsImpl.m
	DEPENDENCIES
		CoreFoundation
		Foundation
		system
)

add_darling_executable(fseventsd fseventsd.m)
target_link_libraries(fseventsd system Foundation CarbonCore)

install(TARGETS fseventsd DESTINATION libexec/darling/usr/sbin)
install(CODE "execute_process(COMMAND ${SETCAP_EXECUTABLE} \"cap_sys_admin+ep\" \"${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/sbin/fseventsd\")")
