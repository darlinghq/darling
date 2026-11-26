project(GeneralPreferencePane)

# General.prefPane (Appearance.prefPane on macOS): appearance, colours, scroll bars and document behaviour
add_compile_options(-nostdinc)

add_darling_prefpane(GeneralPane
	BUNDLE_NAME Appearance
	INFO_PLIST Info.plist
	SOURCES
		GeneralPane.m
		../Common/PKPane.m
	LIBRARIES
		AppKit
		Foundation
		CoreServices
		PreferencePanes
		CoreFoundation
		objc
		system
	ICONSET General.iconset
)
set_source_files_properties(GeneralPane.m ../Common/PKPane.m PROPERTIES COMPILE_FLAGS -fobjc-arc)
target_include_directories(GeneralPane PRIVATE ../Common)
