project(AudioToolbox)

include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src/external/libcxx/include ${CMAKE_CURRENT_BINARY_DIR})
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	${CMAKE_CURRENT_SOURCE_DIR}/../include/AudioToolbox
)

add_definitions(-DENABLE_PULSEAUDIO=1)

set(audiotoolbox_sources
	AUComponent.cpp
	AudioConverter.cpp
	AudioConverterImpl.cpp
	AudioFile.cpp
	AudioOutputUnitComponent.cpp
	AudioQueueBase.cpp
	AudioQueue.cpp
	AudioQueueOutput.cpp
	# AudioUnitALSA.cpp
	AudioUnitBase.cpp
	AudioUnit.cpp
	AudioUnitPA.cpp
	AudioUnitRenderer.cpp
	AUGraph.cpp
)

add_framework(AudioToolbox
	FAT
	CURRENT_VERSION
    VERSION "A"

    SOURCES
        ${audiotoolbox_sources}

    DEPENDENCIES
        system
        cxx
		CoreFoundation
		CoreAudio
		CoreServices
		objc
		avformat
		avresample
		avcodec
		avutil
		pulse # This should go away in favor of abstraction inside CoreAudio
)

# TODO: Add AudioUnit.framework
