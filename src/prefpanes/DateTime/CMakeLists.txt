project(DateTimePreferencePane)

# Date & Time.prefPane
add_compile_options(-nostdinc)

add_darling_prefpane(DateTimePane
	BUNDLE_NAME DateAndTime
	INFO_PLIST Info.plist
	SOURCES
		DateTimePane.m
		../Common/PKPane.m
	LIBRARIES
		AppKit
		Foundation
		CoreServices
		PreferencePanes
		CoreFoundation
		objc
		system
	ICONSET DateAndTime.iconset
)
set_source_files_properties(DateTimePane.m ../Common/PKPane.m PROPERTIES COMPILE_FLAGS -fobjc-arc)
target_include_directories(DateTimePane PRIVATE ../Common)
