project(DarlingPreferencePane)

# Darling.prefPane: the settings of Darling in System Preferences
add_compile_options(-nostdinc)

add_darling_prefpane(DarlingPane
	BUNDLE_NAME Darling
	INFO_PLIST Info.plist
	SOURCES
		DarlingPane.m
		DPSection.m
		DPSetting.m
		DPSound.m
	LIBRARIES
		AppKit
		Foundation
		AudioToolbox
		CoreAudio
		PreferencePanes
		CoreFoundation
		objc
		system
	ICONSET DarlingPane.iconset
)
set_source_files_properties(DarlingPane.m DPSection.m DPSetting.m DPSound.m PROPERTIES COMPILE_FLAGS -fobjc-arc)
