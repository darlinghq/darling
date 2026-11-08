# Signs darlingserver's code signature fixtures (made with Apple's `codesign` on macOS, see
# their README) with adhoc-sign and checks the result with adhoc-sign-check.
#
# cmake -DSIGN=<adhoc-sign> -DCHECK=<adhoc-sign-check> -DFIXTURES=<dir> -DWORK=<dir> -P fixtures.cmake

function(run expect)
	execute_process(COMMAND ${ARGN} RESULT_VARIABLE result)
	if (expect STREQUAL "pass" AND NOT result EQUAL 0)
		message(FATAL_ERROR "failed (${result}): ${ARGN}")
	elseif (expect STREQUAL "fail" AND result EQUAL 0)
		message(FATAL_ERROR "succeeded, but should have failed: ${ARGN}")
	endif()
endfunction()

function(same a b)
	execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files ${a} ${b} RESULT_VARIABLE result)
	if (NOT result EQUAL 0)
		message(FATAL_ERROR "${a} differs from ${b}")
	endif()
endfunction()

file(REMOVE_RECURSE ${WORK})
file(MAKE_DIRECTORY ${WORK})
foreach (f unsigned fat-adhoc fat-runtime make-fixtures.sh)
	file(COPY ${FIXTURES}/${f} DESTINATION ${WORK})
endforeach()

# an unsigned arm64 executable gets a signature
run(fail ${CHECK} ${WORK}/unsigned)
run(pass ${SIGN} ${WORK}/unsigned)
run(pass ${CHECK} --expect-identifier=unsigned ${WORK}/unsigned)

# a universal file (x86_64 and arm64) gets one in each slice; an existing hardened-runtime
# signature is replaced
run(pass ${SIGN} --identifier=org.darlinghq.test.fat ${WORK}/fat-adhoc ${WORK}/fat-runtime)
run(pass ${CHECK} --expect-identifier=org.darlinghq.test.fat ${WORK}/fat-adhoc ${WORK}/fat-runtime)

# re-signing `codesign -s -` output with the same identifier reproduces it byte for byte:
# the layout (CodeDirectory version, page sizes, special slots, blobs, padding, slice
# alignment) is the one `codesign` writes
same(${WORK}/fat-adhoc ${FIXTURES}/fat-adhoc)

# signing again changes nothing
file(COPY ${WORK}/unsigned DESTINATION ${WORK}/again)
run(pass ${SIGN} --identifier=unsigned ${WORK}/again/unsigned)
same(${WORK}/again/unsigned ${WORK}/unsigned)

# a script is left alone
run(pass ${SIGN} ${WORK}/make-fixtures.sh)
same(${WORK}/make-fixtures.sh ${FIXTURES}/make-fixtures.sh)
run(fail ${CHECK} ${WORK}/make-fixtures.sh)
run(pass ${CHECK} --images-only ${WORK}/make-fixtures.sh)
