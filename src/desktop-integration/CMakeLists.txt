project(darling-desktop-integration)

# Host-side desktop integration: the generic Mach-O/application-bundle handlers and the generator for
# desktop entries describing applications installed in a prefix. They run on Linux, not in the guest.
set(Darling_desktop_entries
	org.darlinghq.Darling.MachOBinary.desktop
	org.darlinghq.Darling.AppBundle.desktop)

foreach(desktop_entry IN LISTS Darling_desktop_entries)
	configure_file(${desktop_entry}.in ${CMAKE_CURRENT_BINARY_DIR}/${desktop_entry} @ONLY)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${desktop_entry} DESTINATION share/applications)
endforeach()

install(FILES org.darlinghq.Darling.xml DESTINATION share/mime/packages)
install(PROGRAMS darling-desktop DESTINATION bin)

find_package(Python3 QUIET COMPONENTS Interpreter)
if (Python3_Interpreter_FOUND)
	add_test(NAME darling_desktop
		COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/darling-desktop.py
			${CMAKE_CURRENT_SOURCE_DIR}/darling-desktop
			${CMAKE_CURRENT_SOURCE_DIR}/org.darlinghq.Darling.xml
			${CMAKE_CURRENT_BINARY_DIR}/org.darlinghq.Darling.MachOBinary.desktop
			${CMAKE_CURRENT_BINARY_DIR}/org.darlinghq.Darling.AppBundle.desktop
			${CMAKE_INSTALL_PREFIX})
endif()
