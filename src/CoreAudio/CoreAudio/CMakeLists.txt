project(CoreAudio)
darling_use_pipewire()

include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(coreaudio_sources
	AudioHardware.cpp
    AudioHardwareImpl.cpp
    AudioHardwareListenerRegistry.cpp
    AudioObjectListenerBlocks.cpp
    AudioHardwareTapping.m
    CATapDescription.m
    HostTime.mm
    pulse/AudioHardwareImplPA.cpp
    pulse/AudioHardwareImplPADevice.cpp
    pulse/PADeviceRegistry.cpp
    pulse/PADeviceInfo.cpp
    pulse/AudioHardwareImplPAInput.cpp
    pulse/AudioHardwareImplPAOutput.cpp
    AudioHardwareStream.cpp
    pulse/AudioHardwareStreamPA.cpp
    pulse/AudioHardwareStreamPAInput.cpp
    pulse/AudioHardwareStreamPAOutput.cpp
    pulse/PADispatchMainLoop.cpp
)

if (HAVE_PIPEWIRE)
    list(APPEND coreaudio_sources
        pipewire/AudioHardwareImplPW.cpp
        pipewire/AudioHardwareStreamPW.cpp
        pipewire/AudioHardwareStreamPWInput.cpp
        pipewire/AudioHardwareStreamPWOutput.cpp
        pipewire/PWConfig.cpp
    )
endif ()

add_framework(CoreAudio
	FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        ${coreaudio_sources}

    DEPENDENCIES
        system
        cxx
        CoreFoundation
        Foundation
        pulse
)

if (HAVE_PIPEWIRE)
    # The PipeWire backend is internal to CoreAudio, so its symbols are not exported.
    set_source_files_properties(
        pipewire/AudioHardwareImplPW.cpp
        pipewire/AudioHardwareStreamPW.cpp
        pipewire/AudioHardwareStreamPWInput.cpp
        pipewire/AudioHardwareStreamPWOutput.cpp
        pipewire/PWConfig.cpp
        PROPERTIES COMPILE_OPTIONS "-fvisibility=hidden"
    )
    target_link_libraries(CoreAudio PRIVATE darling_pipewire_client)
endif ()
