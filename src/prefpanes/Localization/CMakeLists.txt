project(LocalizationPreferencePane)

# Language & Region.prefPane
add_compile_options(-nostdinc)

add_darling_prefpane(LocalizationPane
	BUNDLE_NAME Localization
	INFO_PLIST Info.plist
	SOURCES
		LocalizationPane.m
		../Common/PKPane.m
	LIBRARIES
		AppKit
		Foundation
		CoreServices
		PreferencePanes
		CoreFoundation
		objc
		system
	ICONSET Localization.iconset
)
set_source_files_properties(LocalizationPane.m ../Common/PKPane.m PROPERTIES COMPILE_FLAGS -fobjc-arc)
target_include_directories(LocalizationPane PRIVATE ../Common)
