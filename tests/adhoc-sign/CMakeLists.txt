# Host tests of src/buildtools/adhoc-sign: its signatures must pass darlingserver's code
# signature checks (the parser and page verification used under the `enforce` policy)

add_executable(adhoc-sign-check check.c)
target_include_directories(adhoc-sign-check PRIVATE ${CMAKE_SOURCE_DIR}/src/external/cctools-port/cctools/include)
target_link_libraries(adhoc-sign-check PRIVATE darlingserver_dc_codesign)

# darlingserver's fixtures, signed by Apple's codesign on macOS
add_test(NAME adhoc_sign_fixtures
	COMMAND ${CMAKE_COMMAND}
		-DSIGN=$<TARGET_FILE:adhoc-sign>
		-DCHECK=$<TARGET_FILE:adhoc-sign-check>
		-DFIXTURES=${CMAKE_SOURCE_DIR}/src/external/darlingserver/tests/fixtures/codesign
		-DWORK=${CMAKE_CURRENT_BINARY_DIR}/fixtures
		-P ${CMAKE_CURRENT_SOURCE_DIR}/fixtures.cmake)

# what the build produced (run after building them): the dynamic linker, a universal dylib
# linked by the compiler driver, an executable, and a framework put together by lipo from
# separately linked slices
add_test(NAME adhoc_sign_outputs
	COMMAND $<TARGET_FILE:adhoc-sign-check>
		$<TARGET_FILE:system_loader>
		$<TARGET_FILE:system_kernel>
		$<TARGET_FILE:launchctl>)
add_test(NAME adhoc_sign_framework
	COMMAND $<TARGET_FILE:adhoc-sign-check> --expect-identifier=CoreFoundation $<TARGET_FILE:CoreFoundation>)

# every image the last install wrote (install_manifest.txt; skipped until something was
# installed from this build directory). Catches images that are built or installed without
# going through use_ld64's signing step. Prebuilt images installed as shipped with a
# certificate signature are accepted; files that are not in the manifest (e.g. leftovers of
# older installations) are not looked at.
add_test(NAME adhoc_sign_installed
	COMMAND sh -c "test -f \"$1\" || exit 77; exec \"$2\" --images-only --any-signer \"--files-from=$1\""
		sh ${CMAKE_BINARY_DIR}/install_manifest.txt $<TARGET_FILE:adhoc-sign-check>)
set_tests_properties(adhoc_sign_installed PROPERTIES SKIP_RETURN_CODE 77)
