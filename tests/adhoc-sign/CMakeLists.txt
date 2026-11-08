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
