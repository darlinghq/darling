# Run with: cmake -P tests/architecture.cmake
# Checks the list of enabled architectures and the per-architecture helpers of cmake/architecture.cmake.
get_filename_component(here "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
include("${here}/../cmake/architecture.cmake")

# Used by the check at the bottom: asking for an architecture nobody described must fail.
if (DEFINED UNKNOWN_ARCH)
	darling_arch_triple("${UNKNOWN_ARCH}" ignored)
	return()
endif()

function(check_archs i386 x86_64 arm64 expected)
	set(TARGET_i386 ${i386})
	set(TARGET_x86_64 ${x86_64})
	set(TARGET_ARM64 ${arm64})
	generate_architecture()
	if (NOT "${DARLING_ARCHS}" STREQUAL "${expected}")
		message(SEND_ERROR "i386=${i386} x86_64=${x86_64} arm64=${arm64}: DARLING_ARCHS = '${DARLING_ARCHS}', expected '${expected}'")
	endif()
endfunction()

check_archs(OFF OFF OFF "")
check_archs(OFF OFF ON "arm64")
check_archs(OFF ON OFF "x86_64")
check_archs(ON ON OFF "i386;x86_64")
check_archs(OFF ON ON "x86_64;arm64")
check_archs(ON ON ON "i386;x86_64;arm64")

function(check_helpers arch triple is_64bit objc_abi)
	darling_arch_triple(${arch} actual_triple)
	darling_arch_is_64bit(${arch} actual_is_64bit)
	darling_arch_objc_abi(${arch} actual_objc_abi)
	if (NOT actual_triple STREQUAL triple)
		message(SEND_ERROR "${arch}: triple '${actual_triple}', expected '${triple}'")
	endif()
	if (NOT actual_is_64bit STREQUAL is_64bit)
		message(SEND_ERROR "${arch}: is_64bit '${actual_is_64bit}', expected '${is_64bit}'")
	endif()
	if (NOT actual_objc_abi STREQUAL objc_abi)
		message(SEND_ERROR "${arch}: objc_abi '${actual_objc_abi}', expected '${objc_abi}'")
	endif()
endfunction()

check_helpers(i386 "i386-apple-darwin20" FALSE "legacy")
check_helpers(x86_64 "x86_64-apple-darwin20" TRUE "modern")
check_helpers(arm64 "aarch64-apple-darwin20" TRUE "modern")

# An architecture that is not described is an error, not an empty answer
execute_process(
	COMMAND ${CMAKE_COMMAND} -DUNKNOWN_ARCH=ppc -P "${CMAKE_CURRENT_LIST_FILE}"
	RESULT_VARIABLE unknown_result
	ERROR_VARIABLE unknown_error
	OUTPUT_QUIET)
if (unknown_result EQUAL 0 OR NOT unknown_error MATCHES "unknown architecture 'ppc'")
	message(SEND_ERROR "an unknown architecture must be rejected (exit ${unknown_result}): ${unknown_error}")
endif()
