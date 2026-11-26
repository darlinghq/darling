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
	PCMConverter.cpp
	TimePitch.cpp
	AudioFile.cpp
	AudioQueueBase.cpp
	AudioQueue.cpp
	AudioServices.cpp
	AudioQueueOutput.cpp
	AudioQueueInput.cpp
	AUGraph.cpp
	AudioComponent.mm
	AudioComponentManager.mm
	AudioCodec.cpp
	AudioFileComponent.cpp
	AudioFileFormatManager.cpp
	ExtendedAudioFile.cpp
	AudioFormat.cpp
	MusicDevice.cpp
	MusicSequence.cpp
	MusicTrack.cpp
	MusicTrackProperties.cpp
	MusicTrackEditing.cpp
	MusicSequenceTime.cpp
	MusicPlayer.cpp
	MusicDestination.cpp
	MusicEventIterator.cpp
	AudioToolbox.cpp
	AudioHardwareService.cpp
	AUParameters.m
	AudioUnitUtilities.cpp
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
		Foundation
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

