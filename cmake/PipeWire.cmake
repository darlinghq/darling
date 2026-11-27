# Optional PipeWire support.
#
# The PipeWire libraries are host libraries, reached through a wrap_elf()
# wrapper just like FFmpeg and PulseAudio. Detection is deliberately soft: a
# Darling tree can still be configured on hosts without PipeWire development
# files, in which case the PipeWire backend and the camera portal backend are
# not built.

include(wrap_elf)

option(ENABLE_PIPEWIRE "Build the PipeWire audio, camera and screen-capture backends when available" ON)

set(HAVE_PIPEWIRE 0)

if (ENABLE_PIPEWIRE)
	find_package(PkgConfig QUIET)
	if (PkgConfig_FOUND)
		pkg_check_modules(PC_PIPEWIRE QUIET libpipewire-0.3)
	endif ()

	if (PC_PIPEWIRE_FOUND)
		find_path(PIPEWIRE_INCLUDE_DIR
			NAMES pipewire/pipewire.h
			HINTS ${PC_PIPEWIRE_INCLUDE_DIRS}
		)
		string(REGEX REPLACE "(^|[ ;])-l" "\\1" _pipewire_link_libraries "${PC_PIPEWIRE_LIBRARIES}")
		find_library(PIPEWIRE_ELF_LIBRARY
			NAMES ${_pipewire_link_libraries} pipewire-0.3 pipewire
			HINTS ${PC_PIPEWIRE_LIBRARY_DIRS}
		)
	endif ()

	if (PIPEWIRE_INCLUDE_DIR AND PIPEWIRE_ELF_LIBRARY)
		set(HAVE_PIPEWIRE 1)
		wrap_elf(pipewire "${PIPEWIRE_ELF_LIBRARY}" OPTIONAL darling_pipewire_available)
		message(STATUS "PipeWire backend: enabled (${PIPEWIRE_ELF_LIBRARY})")
	else ()
		message(STATUS "PipeWire backend: disabled (development files not found)")
	endif ()
endif ()

# Components that use the PipeWire backends call this from their own directory: the
# compile definition and the host include directories are scoped to them instead of
# being added to every translation unit of the tree.
macro(darling_use_pipewire)
	if (HAVE_PIPEWIRE)
		include_directories(${PIPEWIRE_INCLUDE_DIR} ${PC_PIPEWIRE_INCLUDE_DIRS})
		add_compile_definitions(HAVE_PIPEWIRE=1)
	endif ()
endmacro()
