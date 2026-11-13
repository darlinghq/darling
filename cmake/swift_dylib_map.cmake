# Redirects the guest paths of the Swift overlay to the dylibs shipped in
# src/external/swift, so ld64 never looks for them on the host.
#
# The SDK's .tbd files and the overlays name their dependencies either by the
# absolute install name (/usr/lib/swift/libswiftX.dylib) or, for the older
# overlays, by @rpath/libswiftX.dylib. Both forms are mapped to the same file.
#
# The map covers:
# 1. Dylibs shipped in the swift submodule's source directory. They are found
#    with a glob, so the list follows the submodule instead of being kept here.
#    A symlink such as libswiftCompatibilitySpan.dylib -> libswiftCore.dylib is
#    mapped like any other entry.
# 2. Dylibs the submodule's CMakeLists.txt builds into the binary directory
#    (the overlays compiled from source, the re-export dylibs for the split
#    Darwin overlay and the WebKit stub). Their output names are fixed by that
#    CMakeLists.txt, so they are listed. The overlays exist only when the build
#    found a Swift toolchain; an entry for a dylib that is not built is harmless,
#    since ld64 only opens the file when a link refers to it.
# 3. Framework bundles (Combine.framework, CryptoKit.framework).

set(DARLING_SWIFT_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/external/swift")

# Dylibs generated in ${CMAKE_BINARY_DIR}/src/external/swift (see the swift
# submodule's CMakeLists.txt).
set(DARLING_SWIFT_GENERATED_DYLIBS
	libswiftDarwin.dylib
	libswiftObjectiveC.dylib
	libswiftCoreFoundation.dylib
	libswiftDispatch.dylib
	libswiftos.dylib
	libswiftXPC.dylib
	libswift_errno.dylib
	libswift_DarwinFoundation1.dylib
	libswift_DarwinFoundation2.dylib
	libswift_DarwinFoundation3.dylib
	libswift_math.dylib
	libswift_signal.dylib
	libswift_stdio.dylib
	libswift_time.dylib
	libswiftsys_time.dylib
	libswiftunistd.dylib
	libswiftWebKit.dylib
)

set(DARLING_SWIFT_FRAMEWORKS
	Combine
	CryptoKit
)

# Builds the whole flag string once; it is identical for every target.
function(_darling_swift_dylib_map_compute out)
	set(map "")

	# CONFIGURE_DEPENDS makes the generator re-run the glob at build time, so a
	# dylib added to the submodule is picked up without a manual re-configure.
	file(GLOB src_dylibs CONFIGURE_DEPENDS "${DARLING_SWIFT_SOURCE_DIR}/libswift*.dylib")
	if (NOT src_dylibs AND EXISTS "${DARLING_SWIFT_SOURCE_DIR}/CMakeLists.txt")
		# An empty list here means every Swift dylib would be unmappable and
		# ld64 would fail much later with "file not found" on each target.
		message(FATAL_ERROR "src/external/swift is present but contains no libswift*.dylib files")
	endif()

	foreach(swift_dylib ${src_dylibs})
		get_filename_component(swift_name "${swift_dylib}" NAME)
		list(FIND DARLING_SWIFT_GENERATED_DYLIBS "${swift_name}" _is_gen)
		if (_is_gen EQUAL -1)
			string(APPEND map " -Wl,-dylib_file,/usr/lib/swift/${swift_name}:${swift_dylib} -Wl,-dylib_file,@rpath/${swift_name}:${swift_dylib}")
		endif()
	endforeach()

	foreach(gen_name ${DARLING_SWIFT_GENERATED_DYLIBS})
		set(gen_path "${CMAKE_BINARY_DIR}/src/external/swift/${gen_name}")
		string(APPEND map " -Wl,-dylib_file,/usr/lib/swift/${gen_name}:${gen_path} -Wl,-dylib_file,@rpath/${gen_name}:${gen_path}")
	endforeach()

	foreach(fw_name ${DARLING_SWIFT_FRAMEWORKS})
		set(fw_binary "${DARLING_SWIFT_SOURCE_DIR}/${fw_name}.framework/Versions/A/${fw_name}")
		if (NOT EXISTS "${fw_binary}")
			set(fw_binary "${CMAKE_BINARY_DIR}/src/external/swift/${fw_name}.framework/Versions/A/${fw_name}")
		endif()
		string(APPEND map " -Wl,-dylib_file,/System/Library/Frameworks/${fw_name}.framework/Versions/A/${fw_name}:${fw_binary} -Wl,-dylib_file,@rpath/${fw_name}.framework/Versions/A/${fw_name}:${fw_binary} -Wl,-dylib_file,@rpath/${fw_name}.framework/${fw_name}:${fw_binary}")
	endforeach()

	set(${out} "${map}" PARENT_SCOPE)
endfunction()

function(add_swift_dylib_map target)
	if (NOT TARGET ${target})
		return()
	endif()

	get_property(map GLOBAL PROPERTY DARLING_SWIFT_DYLIB_MAP)
	if (NOT map)
		_darling_swift_dylib_map_compute(map)
		set_property(GLOBAL PROPERTY DARLING_SWIFT_DYLIB_MAP "${map}")
	endif()

	if (map)
		set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " ${map}")
	endif()
endfunction()
