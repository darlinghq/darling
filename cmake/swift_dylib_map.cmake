# Redirects the guest paths of the Swift overlay to the dylibs built under
# src/external/swift, so ld64 never looks for them on the host.
#
# The list is derived from the directory rather than written out: the submodule
# ships far more libraries than any hand-maintained list kept up with, and every
# one that was missed failed to link with
#   ld: file not found: /usr/lib/swift/<name>.dylib for architecture arm64

FUNCTION(add_swift_dylib_map target)
	if (NOT TARGET ${target})
		return()
	endif()

	file(GLOB DARLING_SWIFT_DYLIBS "${CMAKE_SOURCE_DIR}/src/external/swift/libswift*.dylib")
	set(SWIFT_DYLIB_MAP "")
	foreach(swift_dylib ${DARLING_SWIFT_DYLIBS})
		get_filename_component(swift_name "${swift_dylib}" NAME)
		set(SWIFT_DYLIB_MAP "${SWIFT_DYLIB_MAP} -Wl,-dylib_file,/usr/lib/swift/${swift_name}:${swift_dylib}")
	endforeach()

	if (SWIFT_DYLIB_MAP)
		set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " ${SWIFT_DYLIB_MAP}")
	endif()
ENDFUNCTION()
