project(tools)

if (COMPONENT_cli)
	add_darling_executable(sw_vers sw_vers.c)
	add_darling_executable(sudo sudo.c)
	add_darling_executable(codesign codesign.c)
	add_darling_executable(dsmemberutil dsmemberutil.c)
	add_darling_executable(softwareupdate softwareupdate.c)
	add_darling_executable(spctl spctl.c)

	install(TARGETS sw_vers sudo codesign dsmemberutil DESTINATION libexec/darling/usr/bin)
	install(TARGETS softwareupdate spctl DESTINATION libexec/darling/usr/sbin)

	target_link_libraries(sw_vers CoreFoundation)
endif()

if (COMPONENT_gui)
	add_darling_executable(open open.m)
	target_link_libraries(open CoreServices Foundation AppKit)
	install(TARGETS open DESTINATION libexec/darling/usr/bin)
endif()
