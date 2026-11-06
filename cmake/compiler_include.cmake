# Detect the location of compiler-specific header files
# such as stdbool.h or xmmintrin.h
function(GetCompilerInclude OutputVar)
	# Use a header that exists on all architectures to find the compiler include path
	if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
		set(_probe_header "stddef.h")
	else()
		set(_probe_header "cpuid.h")
	endif()

	file(WRITE "${CMAKE_BINARY_DIR}/cinctest.c" "#include <${_probe_header}>")
	execute_process(COMMAND "${CMAKE_C_COMPILER}" -M "${CMAKE_BINARY_DIR}/cinctest.c"
		RESULT_VARIABLE BuildResult
		OUTPUT_VARIABLE BuildOutput
	)

	if (BuildResult)
		message(FATAL_ERROR "Cannot detect compiler header include path")
	endif (BuildResult)

	separate_arguments(BuildOutput)

	foreach (str ${BuildOutput})
		string(REGEX REPLACE "\n$" "" str "${str}")

		if (str MATCHES "${_probe_header}$")
			string(REGEX REPLACE "${_probe_header}" "" IncPath "${str}")
		endif ()
	endforeach (str)

	if (NOT IncPath)
		message(FATAL_ERROR "Cannot parse compiler output to detect include path")
	endif (NOT IncPath)

	set(${OutputVar} ${IncPath} PARENT_SCOPE)
endfunction(GetCompilerInclude)

