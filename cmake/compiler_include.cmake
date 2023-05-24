# Detect the location of compiler-specific header files
# such as stdbool.h or xmmintrin.h
function(GetCompilerInclude OutputVar)
	if (TARGET_x86_64 OR TARGET_i386)
		set(COMPILER_INCLUDE_HEADER_NAME "cupid")
	else()
		set(COMPILER_INCLUDE_HEADER_NAME "stdbool")
	endif ()

	file(WRITE "${CMAKE_BINARY_DIR}/cinctest.c" "#include <${COMPILER_INCLUDE_HEADER_NAME}.h>")
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
		# message(STATUS "Output: ${str}")

		if (str MATCHES "${COMPILER_INCLUDE_HEADER_NAME}.h$")
			# message(STATUS "Str matched: ${str}")
			string(REGEX REPLACE "${COMPILER_INCLUDE_HEADER_NAME}\\.h" "" IncPath "${str}")
		endif (str MATCHES "${COMPILER_INCLUDE_HEADER_NAME}.h$")
	endforeach (str)

	if (NOT IncPath)
		message(FATAL_ERROR "Cannot parse compiler output to detect include path")
	endif (NOT IncPath)

	set(${OutputVar} ${IncPath} PARENT_SCOPE)
endfunction(GetCompilerInclude)

