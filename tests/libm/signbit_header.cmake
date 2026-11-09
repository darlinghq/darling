# Copyright (C) 2026 Lubos Dolezel
#
# Host test: compile signbit_probe.c against the SDK's math.h for each Darwin
# ABI, in C and C++, and check that the optimizer folds every probe to the
# right constant. The probes cannot run on the host, so the folded LLVM IR is
# the result.
#
# cmake -DCOMPILER=<clang> -DSDK=<MacOSX.sdk> -DPROBE=<signbit_probe.c>
#       -DWORK_DIR=<dir> -P signbit_header.cmake

foreach(var COMPILER SDK PROBE WORK_DIR)
	if (NOT DEFINED ${var})
		message(FATAL_ERROR "${var} is not set")
	endif()
endforeach()

set(expected
	probe_negative 1
	probe_positive 0
	probe_negative_zero 1
	probe_positive_zero 0
	probe_negative_inf 1
)

file(MAKE_DIRECTORY "${WORK_DIR}")
set(failed FALSE)

foreach(target arm64-apple-macos11 x86_64-apple-macos10.15 i386-apple-macos10.13)
	foreach(lang c c++)
		set(ir_file "${WORK_DIR}/signbit-${target}-${lang}.ll")
		execute_process(
			COMMAND "${COMPILER}" -x ${lang} -target ${target} -isysroot "${SDK}"
				-O2 -S -emit-llvm "${PROBE}" -o "${ir_file}"
			RESULT_VARIABLE result
			ERROR_VARIABLE errors
		)
		if (NOT result EQUAL 0)
			message(SEND_ERROR "${target} ${lang}: compile failed:\n${errors}")
			set(failed TRUE)
			continue()
		endif()

		file(READ "${ir_file}" ir)
		set(pairs ${expected})
		while (pairs)
			list(POP_FRONT pairs name want)
			string(REGEX MATCH "@${name}\\(\\)[^}]*ret i32 ([0-9]+)" match "${ir}")
			set(got "${CMAKE_MATCH_1}")
			if (NOT got STREQUAL want)
				message(SEND_ERROR "FAIL ${target} ${lang}: ${name} = '${got}', expected ${want}")
				set(failed TRUE)
			endif()
		endwhile()
	endforeach()
endforeach()

if (failed)
	message(FATAL_ERROR "signbit(long double) is wrong for at least one ABI")
endif()
message(STATUS "PASS: signbit(long double) on arm64, x86_64 and i386, C and C++")
