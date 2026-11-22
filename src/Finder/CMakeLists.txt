project(Finder)

# Finder.app shows folders and volumes in windows: LaunchServices hands it what it declares in its Info.plist
# (folders and volumes), and NSWorkspace sends it reveal requests. It needs AppKit, so it is built with the GUI.
if (NOT COMPONENT_gui)
	return()
endif()

add_compile_options(-nostdinc)

set(Finder_sources
	main.m
	FinderAppDelegate.m
	FinderDirectory.m
	FinderDrop.m
	FinderDSStore.m
	FinderFileOperation.m
	FinderFolderStyle.m
	FinderFreeformLayout.m
	FinderIconViewController.m
	FinderInfoPanelController.m
	FinderItemActions.m
	FinderListViewController.m
	FinderOperationQueue.m
	FinderSidebarController.m
	FinderWindowController.m
)
add_darling_executable(Finder ${Finder_sources})
set_source_files_properties(${Finder_sources} PROPERTIES COMPILE_FLAGS -fobjc-arc)
target_link_libraries(Finder AppKit Foundation CoreServices ImageIO objc)

set(Finder_contents "libexec/darling/System/Library/CoreServices/Finder.app/Contents")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/PkgInfo "APPL????")

# The icon: the .iconset becomes an .icns, as iconutil does in Xcode builds
set(Finder_icns ${CMAKE_CURRENT_BINARY_DIR}/Finder.icns)
file(GLOB Finder_icon_images ${CMAKE_CURRENT_SOURCE_DIR}/Finder.iconset/*.png)
add_custom_command(OUTPUT ${Finder_icns}
	COMMAND $<TARGET_FILE:iconset2icns> ${CMAKE_CURRENT_SOURCE_DIR}/Finder.iconset ${Finder_icns}
	DEPENDS ${Finder_icon_images}
	COMMENT "Creating Finder.icns")
add_custom_target(Finder_icons ALL DEPENDS ${Finder_icns})
add_dependencies(Finder_icons iconset2icns)
add_dependencies(Finder Finder_icons)

install(TARGETS Finder DESTINATION ${Finder_contents}/MacOS COMPONENT gui)
install(FILES Info.plist ${CMAKE_CURRENT_BINARY_DIR}/PkgInfo DESTINATION ${Finder_contents} COMPONENT gui)
install(FILES ${Finder_icns} DESTINATION ${Finder_contents}/Resources COMPONENT gui)
