
include(FindPackageHandleStandardArgs)

find_path(PULSEAUDIO_INCLUDE_DIRS
	NAMES pulse/pulseaudio.h
	DOC "PulseAudio include directory"
)

find_library(PULSEAUDIO_LIBRARIES
	NAMES pulse
	DOC "PulseAudio library"
)

find_package_handle_standard_args(PulseAudio
	REQUIRED_VARS PULSEAUDIO_LIBRARIES PULSEAUDIO_INCLUDE_DIRS VERSION_VAR PULSEAUDIO_VERSION
	FAIL_MESSAGE "Could NOT find PulseAudio!")

mark_as_advanced(PULSEAUDIO_INCLUDE_DIRS PULSEAUDIO_LIBRARIES)
