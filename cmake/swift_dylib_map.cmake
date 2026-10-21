# Redirects the guest paths of the Swift overlay to the dylibs built under
# src/external/swift, so ld64 never looks for them on the host.
#
# The list covers:
# 1. Source dylibs pre-staged in src/external/swift/
# 2. Generated dylibs built under ${CMAKE_BINARY_DIR}/src/external/swift/
#    (Source of truth: src/external/swift/CMakeLists.txt)
# 3. Framework bundles (Combine.framework, CryptoKit.framework)

FUNCTION(add_swift_dylib_map target)
	if (NOT TARGET ${target})
		return()
	endif()

	set(SWIFT_DYLIB_MAP "")

	# Generated dylibs built in binary directory (defined in src/external/swift/CMakeLists.txt).
	# Defined explicitly so ld64 mappings are unconditionally present even on clean builds.
	set(SWIFT_GENERATED_DYLIBS
		libswiftCoreCompat.dylib
		libswiftWebKit.dylib
		libswift_errno.dylib
		libswiftUniformTypeIdentifiers.dylib
	)
	foreach(gen_name ${SWIFT_GENERATED_DYLIBS})
		set(gen_path "${CMAKE_BINARY_DIR}/src/external/swift/${gen_name}")
		set(SWIFT_DYLIB_MAP "${SWIFT_DYLIB_MAP} -Wl,-dylib_file,/usr/lib/swift/${gen_name}:${gen_path} -Wl,-dylib_file,@rpath/${gen_name}:${gen_path}")
	endforeach()

	# Pre-staged dylibs in source tree (skipping any generated dylibs to prevent duplicate mappings)
	file(GLOB DARLING_SWIFT_SRC_DYLIBS "${CMAKE_SOURCE_DIR}/src/external/swift/libswift*.dylib")
	foreach(swift_dylib ${DARLING_SWIFT_SRC_DYLIBS})
		get_filename_component(swift_name "${swift_dylib}" NAME)
		list(FIND SWIFT_GENERATED_DYLIBS "${swift_name}" _is_gen)
		if (_is_gen EQUAL -1)
			set(SWIFT_DYLIB_MAP "${SWIFT_DYLIB_MAP} -Wl,-dylib_file,/usr/lib/swift/${swift_name}:${swift_dylib} -Wl,-dylib_file,@rpath/${swift_name}:${swift_dylib}")
		endif()
	endforeach()

	# Framework bundles (Combine.framework, CryptoKit.framework).
	# Emitted unconditionally for ld64 so clean builds before framework compile succeed.
	set(SWIFT_FRAMEWORKS
		Combine
		CryptoKit
	)
	foreach(fw_name ${SWIFT_FRAMEWORKS})
		set(fw_binary "${CMAKE_SOURCE_DIR}/src/external/swift/${fw_name}.framework/Versions/A/${fw_name}")
		if (NOT EXISTS "${fw_binary}")
			set(fw_binary "${CMAKE_BINARY_DIR}/src/external/swift/${fw_name}.framework/Versions/A/${fw_name}")
		endif()
		set(SWIFT_DYLIB_MAP "${SWIFT_DYLIB_MAP} -Wl,-dylib_file,/System/Library/Frameworks/${fw_name}.framework/Versions/A/${fw_name}:${fw_binary} -Wl,-dylib_file,@rpath/${fw_name}.framework/Versions/A/${fw_name}:${fw_binary} -Wl,-dylib_file,@rpath/${fw_name}.framework/${fw_name}:${fw_binary}")
	endforeach()

	if (SWIFT_DYLIB_MAP)
		set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " ${SWIFT_DYLIB_MAP}")
	endif()
ENDFUNCTION()
