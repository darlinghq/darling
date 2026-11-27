project(libpipewire)
darling_use_pipewire()

# Shared client code for the PipeWire audio backend, the Camera portal capture
# path and ScreenCast-backed screen capture. PipeWire itself is a wrapped host
# library (see cmake/PipeWire.cmake).
add_darling_static_library(darling_pipewire_client FAT
	SOURCES
		camera_portal.c
		portal_common.c
		screencast_portal.c
		screen_capture_backend.c
		pipe_client.c
		pipe_volume.c
)

target_include_directories(darling_pipewire_client BEFORE PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/compat
)

target_include_directories(darling_pipewire_client PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

# libspa's inline string helpers use newlocale/uselocale; Darling's global
# -D_DARWIN_C_SOURCE otherwise hides those declarations from glibc.
target_compile_definitions(darling_pipewire_client PRIVATE _GNU_SOURCE)

# The client is linked statically into each framework that uses it; its functions are
# not part of any framework's interface, so they are not exported from the dylibs.
target_compile_options(darling_pipewire_client PRIVATE -fvisibility=hidden)

target_link_libraries(darling_pipewire_client PUBLIC
	darling_dbus_loader
	pipewire
)

if (Python3_Interpreter_FOUND)
	add_test(NAME camera_portal_host
		COMMAND ${Python3_EXECUTABLE}
			${CMAKE_CURRENT_SOURCE_DIR}/tests/camera_portal_host.py ${CMAKE_SOURCE_DIR})
	add_test(NAME camera_frame_conversion_host
		COMMAND ${Python3_EXECUTABLE}
			${CMAKE_CURRENT_SOURCE_DIR}/tests/camera_frame_host.py ${CMAKE_SOURCE_DIR})
	add_test(NAME screencast_portal_host
		COMMAND ${Python3_EXECUTABLE}
			${CMAKE_CURRENT_SOURCE_DIR}/tests/screencast_portal_host.py ${CMAKE_SOURCE_DIR})
	add_test(NAME screen_capture_conversion_host
		COMMAND ${Python3_EXECUTABLE}
			${CMAKE_CURRENT_SOURCE_DIR}/tests/screen_capture_conversion_host.py ${CMAKE_SOURCE_DIR})
	add_test(NAME screen_input_contract_host
		COMMAND ${Python3_EXECUTABLE}
			${CMAKE_CURRENT_SOURCE_DIR}/tests/screen_input_contract_host.py ${CMAKE_SOURCE_DIR})
endif ()
