project(CoreAudio_root)
include(wrap_elf)

remove_sdk_framework(AudioToolbox)
remove_sdk_framework(AudioUnit)
remove_sdk_framework(CoreAudio)

add_definitions(-DENABLE_PULSEAUDIO)

find_package(FFmpeg REQUIRED)
find_package(PulseAudio REQUIRED)

include_directories(
	${FFMPEG_INCLUDE_DIR}
	${PULSEAUDIO_INCLUDE_DIRS}
)
add_definitions(-DHAVE_AV_FRAME_ALLOC=1)

wrap_elf(swresample libswresample.so)
wrap_elf(avcodec libavcodec.so)
wrap_elf(avformat libavformat.so)
wrap_elf(avutil libavutil.so)
#wrap_elf(asound libasound.so)
wrap_elf(pulse libpulse.so)

set(FRAMEWORK_AUDIOTOOLBOX_VERSION "A")
set(FRAMEWORK_AUDIOUNIT_VERSION "A")
set(FRAMEWORK_COREAUDIO_VERSION "A")

add_subdirectory(CoreAudio)
add_subdirectory(AudioToolbox)
add_subdirectory(CoreAudioComponent)
add_subdirectory(AFAVFormatComponent)
add_subdirectory(AudioFileTools)

generate_sdk_framework(AudioToolbox
    VERSION ${FRAMEWORK_AUDIOTOOLBOX_VERSION}
    HEADER "include/AudioToolbox"
)

generate_sdk_framework(AudioUnit
    VERSION ${FRAMEWORK_AUDIOUNIT_VERSION}
    HEADER "include/AudioUnit"
)

generate_sdk_framework(CoreAudio
    VERSION ${FRAMEWORK_COREAUDIO_VERSION}
    HEADER "include/CoreAudio"
)