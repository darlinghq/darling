project(CoreAudioComponent)
include(darling_bundle)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
add_definitions(-DCA_BASIC_AU_FEATURES=1)
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src/external/libcxx/include ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/PublicUtility
	${CMAKE_CURRENT_SOURCE_DIR}/AUPublic/AUBase
	${CMAKE_CURRENT_SOURCE_DIR}/AUPublic/Utility
)

set(component_sources
	AUPublic/AUBase/AUBase.cpp
	AUPublic/AUBase/AUDispatch.cpp
	AUPublic/AUBase/AUInputElement.cpp
	AUPublic/AUBase/AUOutputElement.cpp
	AUPublic/AUBase/AUPlugInDispatch.cpp
	AUPublic/AUBase/AUScopeElement.cpp
	AUPublic/AUBase/ComponentBase.cpp
	AUPublic/Utility/AUBuffer.cpp
	PublicUtility/CAAudioChannelLayout.cpp
	PublicUtility/CAAudioChannelLayoutObject.cpp
	PublicUtility/CABufferList.cpp
	PublicUtility/CADebugger.cpp
	PublicUtility/CAStreamBasicDescription.cpp
	PublicUtility/CAHostTimeBase.cpp
	PublicUtility/CAVectorUnit.cpp
	AUHAL.cpp
)

add_darling_bundle(CoreAudioComponent "" ${component_sources})
target_link_libraries(CoreAudioComponent
	AudioToolbox
	CoreAudio
	cxx
	system
	CoreFoundation
)
set_target_properties(CoreAudioComponent PROPERTIES OUTPUT_NAME "CoreAudio" PREFIX "" SUFFIX "")
install(TARGETS CoreAudioComponent DESTINATION libexec/darling/System/Library/Components/CoreAudio.component/Contents/MacOS)
install(FILES Info.plist DESTINATION libexec/darling/System/Library/Components/CoreAudio.component/Contents)
