set(SUBFRAMEWORK_VERSION "A")

# LSHostOpenBus.c talks to the host's libdbus-1 (through the wrapper in
# src/native) and needs its headers.
find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_DBUS REQUIRED dbus-1)
include_directories(${PC_DBUS_INCLUDE_DIRS})

generate_sdk_subframework(LaunchServices
	BASE_PATH ${coreservices_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "../../include/LaunchServices"
)

add_framework(LaunchServices
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "CoreServices"

	SOURCES
		IconsCore.c
		LaunchServices.c
		LaunchServices.cpp
		LSApplicationProxy.m
		LSApplicationWorkspace.m
		LSRunning.m
		constants.c
		LSHostOpen.c
		LSHostOpenURL.c
		LSHostOpenBus.c
		LSInfo.m
		LSOpen.m
		UTType.m

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
