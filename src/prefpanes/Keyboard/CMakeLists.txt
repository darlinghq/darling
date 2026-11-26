project(KeyboardPreferencePane)

# Keyboard.prefPane
add_compile_options(-nostdinc)

add_darling_prefpane(KeyboardPane
	BUNDLE_NAME Keyboard
	INFO_PLIST Info.plist
	SOURCES
		KeyboardPane.m
		../Common/PKPane.m
	LIBRARIES
		AppKit
		Foundation
		CoreServices
		PreferencePanes
		CoreFoundation
		objc
		system
	ICONSET Keyboard.iconset
)
set_source_files_properties(KeyboardPane.m ../Common/PKPane.m PROPERTIES COMPILE_FLAGS -fobjc-arc)
target_include_directories(KeyboardPane PRIVATE ../Common)
