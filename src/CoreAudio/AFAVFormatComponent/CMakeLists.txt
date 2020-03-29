project(AFAVFormatComponent)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src/external/libcxx/include ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/PublicUtility
	${CMAKE_CURRENT_SOURCE_DIR}/AFPublic
	${CMAKE_CURRENT_SOURCE_DIR}/AUPublic/AUBase
)

set(component_sources
	AFPublic/AudioFileComponentBase.cpp
	AFPublic/AudioFileFormat.cpp
	AFPublic/AudioFileObject.cpp
	AFPublic/CompressedPacketTable.cpp
	AFPublic/DataSource.cpp
	AUPublic/AUBase/ComponentBase.cpp
	PublicUtility/CACFDictionary.cpp
)

add_darling_bundle(AFAVFormatComponent "" ${component_sources})
target_link_libraries(AFAVFormatComponent
	AudioToolbox
	cxx
	system
	CoreFoundation
	avformat
)
set_target_properties(AFAVFormatComponent PROPERTIES OUTPUT_NAME "AFAVFormat" PREFIX "" SUFFIX "")
install(TARGETS AFAVFormatComponent DESTINATION libexec/darling/System/Library/Components/AFAVFormat.component/Contents/MacOS)
#install(FILES Info.plist DESTINATION libexec/darling/System/Library/Components/AFAVFormat.component/Contents)
