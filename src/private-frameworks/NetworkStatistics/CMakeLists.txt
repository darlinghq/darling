project(NetworkStatistics)

remove_sdk_framework(NetworkStatistics
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(NW_C
	src/functions.c
	src/globals.c
)
set(NW_OBJC
	src/NWRouteSnapshot.m
	src/NWRouteSnapshotter.m
	src/NWSnapshot.m
	src/NWSnapshotSource.m
	src/NWSnapshotter.m
	src/NWStatisticsDelegateBlockWrapper.m
	src/NWStatisticsInterfaceSource.m
	src/NWStatisticsManager.m
	src/NWStatisticsRouteSource.m
	src/NWStatisticsSource.m
	src/NWStatisticsTCPSource.m
	src/NWStatisticsUDPSource.m
)

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(NetworkStatistics
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/NetworkStatistics"
	PRIVATE
)

add_framework(NetworkStatistics
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		${NW_C}
		${NW_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
