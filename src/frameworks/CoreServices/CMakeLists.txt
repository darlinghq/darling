project(CoreServices)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src/external/libcxx/include)
include_directories(
	${CMAKE_SOURCE_DIR}/src/kernel/emulation/linux
)

set(CoreServices_SRCS
	CoreEndian.cpp
	Multiprocessing.cpp
	TextUtils.cpp
	MacMemory.cpp
	MacLocales.cpp
	UnicodeUtilities.cpp
	Gestalt.cpp
	FileManager.cpp
	DateTimeUtils.cpp
	FixMath.cpp
	ToolUtils.cpp
	Math64.cpp
	DriverSynchronization.cpp
	DriverServices.cpp
	Components.cpp
	Processes.cpp
	LaunchServices.cpp
	OpenTransport.cpp
	Timer.cpp
	CoreServicesPriv.cpp
	TextCommon.cpp
	TextEncodingConverter.cpp
	MDQuery.c
	LSApplicationProxy.m
	LSApplicationWorkspace.m
	constants.m
	ComponentManager.cpp
	Files.cpp
	Resources.cpp
	ResourcesImpl.cpp
	MacErrors.cpp
	UserBreak.cpp
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1239.200.12")
add_framework(FSEvents
	FAT
	CURRENT_VERSION
	VERSION "A"
	PARENT "CoreServices"
	SOURCES
		src/FSEvents/FSEvents.m
		src/FSEvents/FSEventsImpl.m
	DEPENDENCIES
		CoreFoundation
		Foundation
		system
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "945.0.0")
add_framework(LaunchServices
	FAT
	CURRENT_VERSION
	VERSION "A"
	PARENT "CoreServices"
	SOURCES
	src/LaunchServices/LaunchServices.c
	DEPENDENCIES
		CoreFoundation
		system
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "945.0.0")
add_framework(CoreServices
	FAT
	CURRENT_VERSION
	VERSION "A"
	SOURCES
		${CoreServices_SRCS}
	DEPENDENCIES
		FSEvents
		LaunchServices
		icucore
		system
		CoreFoundation
		cxx
		iconv
		Foundation
		${EXTRA_LIBS}
	RESOURCES
		Info.plist Info.plist
)

set(FSEvents_BUILD ${CMAKE_CURRENT_BINARY_DIR}/FSEvents)
set(LaunchServices_BUILD ${CMAKE_CURRENT_BINARY_DIR}/LaunchServices)
set(CFNetwork_BUILD ${CMAKE_BINARY_DIR}/src/external/cfnetwork/src/CFNetwork)
set(CoreFoundation_BUILD ${CMAKE_BINARY_DIR}/src/external/corefoundation/CoreFoundation)
reexport(CoreServices FSEvents ${FSEvents_BUILD})
reexport(CoreServices LaunchServices ${LaunchServices_BUILD})
reexport(CoreServices CFNetwork ${CFNetwork_BUILD})
reexport(CoreServices CoreFoundation ${CoreFoundation_BUILD})

install(FILES SystemVersion.plist DESTINATION "libexec/darling/System/Library/CoreServices")
