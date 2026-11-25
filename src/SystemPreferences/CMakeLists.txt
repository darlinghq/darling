project(SystemPreferences)

# System Preferences.app hosts the .prefPane bundles of /System/Library/PreferencePanes,
# /Library/PreferencePanes and ~/Library/PreferencePanes (through PreferencePanes.framework),
# opens .prefPane documents and handles x-apple.systempreferences: URLs.
if (NOT COMPONENT_gui)
	return()
endif()

add_compile_options(-nostdinc)

add_darling_executable(SystemPreferences
	main.m
	SPAppDelegate.m
	SPGridView.m
	SPPane.m
	SPPaneCatalog.m
	SPWindowController.m
)
set_source_files_properties(
	main.m SPAppDelegate.m SPGridView.m SPPane.m SPPaneCatalog.m SPWindowController.m
	PROPERTIES COMPILE_FLAGS -fobjc-arc)
target_link_libraries(SystemPreferences AppKit Foundation PreferencePanes objc)
set_target_properties(SystemPreferences PROPERTIES OUTPUT_NAME "System Preferences")

set(SystemPreferences_contents "libexec/darling/Applications/System Preferences.app/Contents")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/PkgInfo "APPL????")

# The icon: the .iconset becomes an .icns, as iconutil does in Xcode builds
set(SystemPreferences_icns ${CMAKE_CURRENT_BINARY_DIR}/PrefApp.icns)
file(GLOB SystemPreferences_icon_images ${CMAKE_CURRENT_SOURCE_DIR}/PrefApp.iconset/*.png)
add_custom_command(OUTPUT ${SystemPreferences_icns}
	COMMAND $<TARGET_FILE:iconset2icns> ${CMAKE_CURRENT_SOURCE_DIR}/PrefApp.iconset ${SystemPreferences_icns}
	DEPENDS ${SystemPreferences_icon_images}
	COMMENT "Creating PrefApp.icns")
add_custom_target(SystemPreferences_icons ALL DEPENDS ${SystemPreferences_icns})
add_dependencies(SystemPreferences_icons iconset2icns)
add_dependencies(SystemPreferences SystemPreferences_icons)

install(TARGETS SystemPreferences DESTINATION ${SystemPreferences_contents}/MacOS COMPONENT gui)
install(FILES Info.plist ${CMAKE_CURRENT_BINARY_DIR}/PkgInfo DESTINATION ${SystemPreferences_contents} COMPONENT gui)
install(FILES ${SystemPreferences_icns} NSPrefPaneGroups.plist DESTINATION ${SystemPreferences_contents}/Resources COMPONENT gui)
