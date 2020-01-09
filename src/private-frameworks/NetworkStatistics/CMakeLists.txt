project(NetworkStatistics)

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

add_framework(NetworkStatistics
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		${NW_C}
		${NW_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
