# Copyright (C) 2026 Lubos Dolezel
#
# Detects the Swift toolchain that builds the Swift SDK overlays (src/external/swift).
#
# The overlays are optional: when no usable toolchain is found, they are skipped with a status message
# and everything else builds as before.
#
# Cache variable:
#   SWIFT_TOOLCHAIN   Root of a Swift toolchain, i.e. the directory that contains usr/bin/swiftc (or the usr
#                     directory itself). Leave empty to search $SWIFT_TOOLCHAIN in the environment,
#                     /opt/swift-*, /usr/local/swift*, and then PATH. Set it to OFF to skip the overlays.
#
# Results:
#   SWIFT_TOOLCHAIN_FOUND     TRUE when a swiftc of at least SWIFT_TOOLCHAIN_MIN_VERSION was found
#   SWIFT_COMPILER            full path of swiftc
#   SWIFT_TOOLCHAIN_USR       the toolchain's usr directory (holds bin/, lib/swift, lib/clang)
#   SWIFT_TOOLCHAIN_VERSION   version reported by swiftc
#
# The compiler only needs to run on the build host: the overlays are cross-compiled for macOS from the
# stdlib module interfaces kept next to the Swift runtime in the swift submodule, not from the toolchain's
# own (Linux) stdlib.

set(SWIFT_TOOLCHAIN "" CACHE STRING
	"Swift toolchain used to build the Swift SDK overlays (root directory, empty to auto-detect, OFF to skip)")
set(SWIFT_TOOLCHAIN_MIN_VERSION "6.3" CACHE INTERNAL "Oldest Swift compiler that can read the shipped stdlib interfaces")

set(SWIFT_TOOLCHAIN_FOUND FALSE)
unset(SWIFT_COMPILER)
unset(SWIFT_TOOLCHAIN_USR)
unset(SWIFT_TOOLCHAIN_VERSION)

macro(_swift_toolchain_skip reason)
	message(STATUS "Swift toolchain: ${reason}; the Swift SDK overlays will not be built")
	set(SWIFT_TOOLCHAIN_FOUND FALSE)
	unset(SWIFT_COMPILER)
	unset(SWIFT_TOOLCHAIN_USR)
	unset(SWIFT_TOOLCHAIN_VERSION)
endmacro()

if (SWIFT_TOOLCHAIN MATCHES "^(OFF|NO|FALSE|0|N)$")
	_swift_toolchain_skip("disabled by SWIFT_TOOLCHAIN=${SWIFT_TOOLCHAIN}")
else()
	# find_program caches its result; a changed SWIFT_TOOLCHAIN must search again.
	if (NOT "${SWIFT_TOOLCHAIN}" STREQUAL "${_SWIFT_TOOLCHAIN_SEARCHED_FOR}")
		unset(SWIFT_COMPILER CACHE)
	endif()
	set(_SWIFT_TOOLCHAIN_SEARCHED_FOR "${SWIFT_TOOLCHAIN}" CACHE INTERNAL "")

	if (SWIFT_TOOLCHAIN)
		find_program(SWIFT_COMPILER NAMES swiftc
			PATHS "${SWIFT_TOOLCHAIN}/usr/bin" "${SWIFT_TOOLCHAIN}/bin"
			NO_DEFAULT_PATH)
	else()
		file(GLOB _swift_candidates "/opt/swift-*/usr/bin" "/usr/local/swift*/usr/bin")
		list(SORT _swift_candidates)
		list(REVERSE _swift_candidates)
		find_program(SWIFT_COMPILER NAMES swiftc
			HINTS ENV SWIFT_TOOLCHAIN
			PATHS ${_swift_candidates})
		unset(_swift_candidates)
	endif()

	if (NOT SWIFT_COMPILER)
		if (SWIFT_TOOLCHAIN)
			_swift_toolchain_skip("no swiftc under SWIFT_TOOLCHAIN=${SWIFT_TOOLCHAIN}")
		else()
			_swift_toolchain_skip("no swiftc found (set SWIFT_TOOLCHAIN to a Swift ${SWIFT_TOOLCHAIN_MIN_VERSION} toolchain)")
		endif()
	else()
		execute_process(COMMAND "${SWIFT_COMPILER}" --version
			RESULT_VARIABLE _swift_rc OUTPUT_VARIABLE _swift_out ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
		if (NOT _swift_rc EQUAL 0)
			_swift_toolchain_skip("${SWIFT_COMPILER} does not run on this host")
		elseif (NOT _swift_out MATCHES "Swift version ([0-9]+(\\.[0-9]+)+)")
			_swift_toolchain_skip("cannot parse the version of ${SWIFT_COMPILER}")
		else()
			set(_swift_version "${CMAKE_MATCH_1}")
			get_filename_component(_swift_bin "${SWIFT_COMPILER}" REALPATH)
			get_filename_component(_swift_bin "${_swift_bin}" DIRECTORY)
			get_filename_component(_swift_usr "${_swift_bin}" DIRECTORY)
			if (_swift_version VERSION_LESS SWIFT_TOOLCHAIN_MIN_VERSION)
				_swift_toolchain_skip("${SWIFT_COMPILER} is Swift ${_swift_version}, ${SWIFT_TOOLCHAIN_MIN_VERSION} or newer is required")
			elseif (NOT EXISTS "${_swift_usr}/lib/swift/shims" OR NOT IS_DIRECTORY "${_swift_usr}/lib/clang")
				_swift_toolchain_skip("${_swift_usr} lacks lib/swift/shims or lib/clang")
			else()
				set(SWIFT_TOOLCHAIN_FOUND TRUE)
				set(SWIFT_TOOLCHAIN_USR "${_swift_usr}")
				set(SWIFT_TOOLCHAIN_VERSION "${_swift_version}")
				message(STATUS "Swift toolchain: ${SWIFT_COMPILER} (Swift ${SWIFT_TOOLCHAIN_VERSION}); the Swift SDK overlays will be built")
			endif()
			unset(_swift_version)
			unset(_swift_bin)
			unset(_swift_usr)
		endif()
		unset(_swift_rc)
		unset(_swift_out)
	endif()
endif()
mark_as_advanced(SWIFT_COMPILER)
