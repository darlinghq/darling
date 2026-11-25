# add_darling_prefpane(<target>
#     BUNDLE_NAME <name>            # <name>.prefPane; also the executable's name (default: the target's name)
#     INFO_PLIST <file>
#     SOURCES <files...>
#     LIBRARIES <libraries...>      # what the pane links with (add_darling_bundle's usual ones are added)
#     [ICONSET <directory>]         # a .iconset folder; becomes <icon name>.icns in Resources
#     [ICON_NAME <name>]            # the icns file's name without the extension (default: the iconset's)
#     [RESOURCES <files...>]        # installed in Contents/Resources as they are
#     [INSTALL_DIR <directory>]     # default: libexec/darling/System/Library/PreferencePanes
#     [COMPONENT <component>])
#
# Builds the sources into the bundle's executable (a Mach-O bundle that System Preferences loads
# with NSBundle; 64-bit slices only, like the application) and installs the .prefPane directory:
#     <INSTALL_DIR>/<name>.prefPane/Contents/{Info.plist,PkgInfo,MacOS/<name>,Resources/...}
# The principal class and the pane's identifier, label, icon (NSPrefPaneIconFile) and search words
# are in the Info.plist, which the caller writes.
include(darling_bundle)

function(add_darling_prefpane target)
	cmake_parse_arguments(PANE "" "BUNDLE_NAME;INFO_PLIST;ICONSET;ICON_NAME;INSTALL_DIR;COMPONENT" "SOURCES;LIBRARIES;RESOURCES" ${ARGN})

	if (NOT PANE_BUNDLE_NAME)
		set(PANE_BUNDLE_NAME ${target})
	endif ()
	if (NOT PANE_INSTALL_DIR)
		set(PANE_INSTALL_DIR libexec/darling/System/Library/PreferencePanes)
	endif ()
	if (NOT PANE_COMPONENT)
		set(PANE_COMPONENT ${CMAKE_INSTALL_DEFAULT_COMPONENT_NAME})
	endif ()
	set(contents ${PANE_INSTALL_DIR}/${PANE_BUNDLE_NAME}.prefPane/Contents)

	# System Preferences is a 64-bit application, so is every pane it loads
	set(DARLING_LIB_64BIT_ONLY TRUE)
	add_darling_bundle(${target} "" ${PANE_SOURCES})
	target_link_libraries(${target} ${PANE_LIBRARIES})
	set_target_properties(${target} PROPERTIES OUTPUT_NAME "${PANE_BUNDLE_NAME}" PREFIX "" SUFFIX "")

	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${target}-PkgInfo "BNDL????")
	install(TARGETS ${target} DESTINATION ${contents}/MacOS COMPONENT ${PANE_COMPONENT})
	install(FILES ${PANE_INFO_PLIST} DESTINATION ${contents} COMPONENT ${PANE_COMPONENT})
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${target}-PkgInfo DESTINATION ${contents} RENAME PkgInfo COMPONENT ${PANE_COMPONENT})
	if (PANE_RESOURCES)
		install(FILES ${PANE_RESOURCES} DESTINATION ${contents}/Resources COMPONENT ${PANE_COMPONENT})
	endif ()

	if (PANE_ICONSET)
		get_filename_component(iconset ${PANE_ICONSET} ABSOLUTE)
		if (NOT PANE_ICON_NAME)
			get_filename_component(PANE_ICON_NAME ${iconset} NAME_WE)
		endif ()
		set(icns ${CMAKE_CURRENT_BINARY_DIR}/${PANE_ICON_NAME}.icns)
		file(GLOB iconset_images ${iconset}/*.png)
		add_custom_command(OUTPUT ${icns}
			COMMAND $<TARGET_FILE:iconset2icns> ${iconset} ${icns}
			DEPENDS ${iconset_images}
			COMMENT "Creating ${PANE_ICON_NAME}.icns")
		add_custom_target(${target}_icons ALL DEPENDS ${icns})
		add_dependencies(${target}_icons iconset2icns)
		add_dependencies(${target} ${target}_icons)
		install(FILES ${icns} DESTINATION ${contents}/Resources COMPONENT ${PANE_COMPONENT})
	endif ()
endfunction()
