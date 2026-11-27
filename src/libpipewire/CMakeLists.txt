project(libpipewire)
darling_use_pipewire()

# One shared client below the media frameworks. Audio, camera, CoreGraphics
# display capture and their callers must observe the same PipeWire connection
# and portal state, so linking this code statically into each dylib would be
# incorrect. PipeWire itself is a wrapped host library (see cmake/PipeWire.cmake).
set(DYLIB_INSTALL_NAME "/usr/lib/darling/libdarling_pipewire_client.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
add_darling_library(darling_pipewire_client SHARED
	camera_portal.c
	portal_common.c
	screencast_portal.c
	screen_capture_backend.c
	pipe_client.c
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

# Only functions declared with DARLING_PW_EXPORT are part of the client's ABI.
target_compile_options(darling_pipewire_client PRIVATE -fvisibility=hidden)

install(TARGETS darling_pipewire_client DESTINATION libexec/darling/usr/lib/darling)

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
	add_test(NAME pipe_client_contract_host
		COMMAND ${Python3_EXECUTABLE}
			${CMAKE_CURRENT_SOURCE_DIR}/tests/pipe_client_contract_host.py ${CMAKE_SOURCE_DIR})
endif ()
