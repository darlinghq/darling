project(DiskImageMounter)

# DiskImageMounter.app opens the disk images LaunchServices hands it (it declares their types in its
# Info.plist): it asks for an image's licence agreement, attaches the image with libdarling-dmg and
# opens its volume. It needs AppKit, so it is built with the GUI.
if (NOT COMPONENT_gui)
	return()
endif()

add_compile_options(-nostdinc)

add_darling_executable(DiskImageMounter
	main.m
	DIMAppDelegate.m
	DIMImageOpener.m
	DIMLicenseWindowController.m
)
set_source_files_properties(main.m DIMAppDelegate.m DIMImageOpener.m DIMLicenseWindowController.m PROPERTIES COMPILE_FLAGS -fobjc-arc)
target_include_directories(DiskImageMounter PRIVATE ${CMAKE_SOURCE_DIR}/src/external/darling-dmg/include)
target_link_libraries(DiskImageMounter darling-dmg AppKit Foundation objc)

set(DiskImageMounter_contents "libexec/darling/System/Library/CoreServices/DiskImageMounter.app/Contents")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/PkgInfo "APPL????")

# The icon: the .iconset becomes an .icns, as iconutil does in Xcode builds
set(DiskImageMounter_icns ${CMAKE_CURRENT_BINARY_DIR}/DiskImageMounter.icns)
file(GLOB DiskImageMounter_icon_images ${CMAKE_CURRENT_SOURCE_DIR}/DiskImageMounter.iconset/*.png)
add_custom_command(OUTPUT ${DiskImageMounter_icns}
	COMMAND $<TARGET_FILE:iconset2icns> ${CMAKE_CURRENT_SOURCE_DIR}/DiskImageMounter.iconset ${DiskImageMounter_icns}
	DEPENDS ${DiskImageMounter_icon_images}
	COMMENT "Creating DiskImageMounter.icns")
add_custom_target(DiskImageMounter_icons ALL DEPENDS ${DiskImageMounter_icns})
add_dependencies(DiskImageMounter_icons iconset2icns)
add_dependencies(DiskImageMounter DiskImageMounter_icons)

install(TARGETS DiskImageMounter DESTINATION ${DiskImageMounter_contents}/MacOS COMPONENT gui)
install(FILES Info.plist ${CMAKE_CURRENT_BINARY_DIR}/PkgInfo DESTINATION ${DiskImageMounter_contents} COMPONENT gui)
install(FILES ${DiskImageMounter_icns} DESTINATION ${DiskImageMounter_contents}/Resources COMPONENT gui)

# The files that let a Linux desktop open disk images with Darling (tests/desktop-integration.py)
find_package(Python3 QUIET COMPONENTS Interpreter)
if (Python3_Interpreter_FOUND)
	add_test(NAME desktop_integration
		COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/desktop-integration.py ${CMAKE_BINARY_DIR} ${CMAKE_INSTALL_PREFIX})
endif()

# The Linux desktop: a desktop entry that opens disk images with "darling open", the MIME type it declares and its
# icon (hicolor theme, the iconset's 1x images)
set(DiskImageMounter_desktop_id org.darlinghq.Darling.DiskImageMounter)
configure_file(linux/${DiskImageMounter_desktop_id}.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/${DiskImageMounter_desktop_id}.desktop @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DiskImageMounter_desktop_id}.desktop DESTINATION share/applications COMPONENT gui)
install(FILES linux/${DiskImageMounter_desktop_id}.xml DESTINATION share/mime/packages COMPONENT gui)
foreach(size 16 32 128 256 512)
	install(FILES DiskImageMounter.iconset/icon_${size}x${size}.png DESTINATION share/icons/hicolor/${size}x${size}/apps
		RENAME ${DiskImageMounter_desktop_id}.png COMPONENT gui)
endforeach()
