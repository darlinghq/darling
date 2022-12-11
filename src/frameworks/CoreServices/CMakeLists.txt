project(CoreServices)

remove_sdk_framework(CoreServices)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

include_directories(
	${CMAKE_SOURCE_DIR}/src/kernel/emulation/linux
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1239.200.12")

set(FRAMEWORK_VERSION "A")

get_path_preframework(coreservices_sdk_path)
append_path_sdk_subframework(${coreservices_sdk_path} coreservices_sdk_path CoreServices
    VERSION ${FRAMEWORK_VERSION}
)

add_subdirectory(src/AE)
add_subdirectory(src/CarbonCore)
add_subdirectory(src/DictionaryServices)
add_subdirectory(src/FSEvents)
add_subdirectory(src/LaunchServices)
add_subdirectory(src/Metadata)
add_subdirectory(src/OSServices)
add_subdirectory(src/SearchKit)
add_subdirectory(src/SharedFileList)

generate_sdk_framework(CoreServices
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CoreServices"
)

add_framework(CoreServices
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		constants.m

	DEPENDENCIES
		FSEvents
		LaunchServices
		CarbonCore
		AE
		DictionaryServices
		Metadata
		SearchKit
		SharedFileList
		OSServices
		system
		CoreFoundation
		${EXTRA_LIBS}

	RESOURCES
		Info.plist Info.plist
)

set(FSEvents_BUILD ${CMAKE_CURRENT_BINARY_DIR}/src/FSEvents/FSEvents)
set(LaunchServices_BUILD ${CMAKE_CURRENT_BINARY_DIR}/src/LaunchServices/LaunchServices)
set(CFNetwork_BUILD ${CMAKE_BINARY_DIR}/src/external/cfnetwork/src/CFNetwork)
set(CoreFoundation_BUILD ${CMAKE_BINARY_DIR}/src/external/corefoundation/CoreFoundation)
set(AE_BUILD ${CMAKE_CURRENT_BINARY_DIR}/src/AE/AE)
set(OSServices_BUILD ${CMAKE_CURRENT_BINARY_DIR}/src/OSServices/OSServices)
set(CarbonCore_BUILD ${CMAKE_CURRENT_BINARY_DIR}/src/CarbonCore/CarbonCore)
set(DictionaryServices_BUILD ${CMAKE_CURRENT_BINARY_DIR}/src/DictionaryServices/DictionaryServices)
set(Metadata_BUILD ${CMAKE_CURRENT_BINARY_DIR}/src/Metadata/Metadata)
set(SearchKit_BUILD ${CMAKE_CURRENT_BINARY_DIR}/src/SearchKit/SearchKit)
set(SharedFileList_BUILD ${CMAKE_CURRENT_BINARY_DIR}/src/SharedFileList/SharedFileList)

reexport(CoreServices FSEvents ${FSEvents_BUILD})
reexport(CoreServices LaunchServices ${LaunchServices_BUILD})
reexport(CoreServices CFNetwork ${CFNetwork_BUILD})
reexport(CoreServices CoreFoundation ${CoreFoundation_BUILD})
reexport(CoreServices AE ${AE_BUILD})
reexport(CoreServices CarbonCore ${CarbonCore_BUILD})
reexport(CoreServices DictionaryServices ${DictionaryServices_BUILD})
reexport(CoreServices Metadata ${Metadata_BUILD})
reexport(CoreServices SearchKit ${SearchKit_BUILD})
reexport(CoreServices SharedFileList ${SharedFileList_BUILD})
reexport(CoreServices OSServices ${OSServices_BUILD})

set_property(TARGET CoreServices APPEND_STRING PROPERTY
  LINK_FLAGS " -Wl,-reexported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/reexport.exp")

install(FILES SystemVersion.plist DESTINATION "libexec/darling/System/Library/CoreServices")
install(FILES SystemVersionCompat.plist DESTINATION "libexec/darling/System/Library/CoreServices")