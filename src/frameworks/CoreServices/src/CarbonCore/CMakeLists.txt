set(CarbonCore_SRCS
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
	OpenTransport.cpp
	Timer.cpp
	TextCommon.cpp
	TextEncodingConverter.cpp
	ComponentManager.cpp
	Files.cpp
	Resources.cpp
	ResourcesImpl.cpp
	MacErrors.cpp
	UserBreak.cpp
	Debugging.cpp
	CoreServicesPriv.cpp
	BackupCore.c
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "945.0.0")

set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(CarbonCore
	BASE_PATH ${coreservices_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "../../include/CarbonCore"
)

add_framework(CarbonCore
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "CoreServices"

	SOURCES 
		${CarbonCore_SRCS}
	
	DEPENDENCIES
		CoreFoundation
		Foundation
		system
		cxx
		icucore
		iconv
)
