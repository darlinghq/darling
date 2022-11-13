project(InternationalSupport)

remove_sdk_framework(InternationalSupport
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "69.1.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(InternationalSupport
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/InternationalSupport"
)


add_framework(InternationalSupport
	PRIVATE
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/ISUtilities.m
		src/NSLocale+InternationalSupportExtensions.m
		src/NSBundle+InternationalSupportExtensions.m

	DEPENDENCIES
		system
		objc
		Foundation
)