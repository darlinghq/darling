project(CoreAudio)

include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src/external/libcxx/include ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(coreaudio_sources
	AudioHardware.cpp
    AudioHardwareImpl.cpp
    AudioHardwareImplPA.cpp
	AudioHardwareStream.cpp
)

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
)
