
include_directories(
	${CMAKE_SOURCE_DIR}/src/external/fmdb/src
)

add_framework(LaunchServices
	FAT
	CURRENT_VERSION
	VERSION "A"
	PARENT "CoreServices"
	SOURCES
		LaunchServices.c
		LaunchServices.cpp
		LSApplicationProxy.m
		LSApplicationWorkspace.m
		LSRunning.m
		constants.c
		LSInfo.m
		LSOpen.m
	DEPENDENCIES
		CoreFoundation
		Foundation
		cxx
		icucore
		system
		CarbonCore
		FMDB
)

add_subdirectory(launchservicesd)
