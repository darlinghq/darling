project(CoreAudio_root)
include(wrap_elf)

add_definitions(-DENABLE_PULSEAUDIO)

find_package(FFmpeg REQUIRED)
find_package(PulseAudio REQUIRED)

include_directories(
	${FFMPEG_INCLUDE_DIR}
	${PULSEAUDIO_INCLUDE_DIRS}
)
add_definitions(-DHAVE_AV_FRAME_ALLOC=1)

wrap_elf(avresample libavresample.so)
wrap_elf(avcodec libavcodec.so)
wrap_elf(avformat libavformat.so)
wrap_elf(avutil libavutil.so)
#wrap_elf(asound libasound.so)
wrap_elf(pulse libpulse.so)

add_subdirectory(CoreAudio)
#add_subdirectory(AudioToolbox)
