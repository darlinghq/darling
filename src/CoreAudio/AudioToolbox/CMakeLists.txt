project(AudioToolbox)

include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	${CMAKE_CURRENT_SOURCE_DIR}/../include/AudioToolbox
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(audiotoolbox_sources
	AUComponent.cpp
	AudioConverter.cpp
	AudioConverterImpl.cpp
	AudioFile.cpp
	AudioQueueBase.cpp
	AudioQueue.cpp
	AudioQueueOutput.cpp
	AUGraph.cpp
	AudioComponent.mm
	AudioComponentManager.mm
	AudioFileComponent.cpp
	AudioFileFormatManager.cpp
	ExtendedAudioFile.cpp
	AudioFormat.cpp
	MusicDevice.cpp
	AudioToolbox.cpp
)

add_framework(AudioToolbox
	FAT
	CURRENT_VERSION
    VERSION ${FRAMEWORK_AUDIOTOOLBOX_VERSION}

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
		swresample
		avcodec
		avutil
)

add_framework(AudioUnit
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_AUDIOUNIT_VERSION}

	SOURCES
		empty.c

	DEPENDENCIES
		AudioToolbox
)
set(AudioToolbox_BUILD ${CMAKE_CURRENT_BINARY_DIR}/AudioToolbox)

reexport(AudioUnit AudioToolbox ${AudioToolbox_BUILD})

