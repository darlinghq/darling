project(CoreAnalytics)

remove_sdk_framework(CoreAnalytics
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CoreAnalytics
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CoreAnalytics"
	PRIVATE
)

add_framework(CoreAnalytics
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/CoreAnalytics.mm
		src/AnalyticsEventObserver.m
		src/AnalyticsConfigurationObserver.m

	DEPENDENCIES
		system
		objc
		Foundation
		cxx
		AppleSauce
)
