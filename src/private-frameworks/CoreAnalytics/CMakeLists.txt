project(CoreAnalytics)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(CoreAnalytics
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

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
