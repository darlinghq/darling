project(PreferencePanesBundles)

# The preference panes that ship with Darling, installed in /System/Library/PreferencePanes
# for System Preferences.app to host.
if (NOT COMPONENT_gui)
	return()
endif()

add_subdirectory(Darling)
add_subdirectory(General)
add_subdirectory(Localization)
add_subdirectory(Keyboard)
add_subdirectory(DateTime)
