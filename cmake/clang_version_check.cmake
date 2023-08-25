set(MANUALLY_SET_COMPILER_ERROR_MESSAGE 
"If you already have a supported version of clang installed, you may need to \
manually set CMAKE_C_COMPILER and CMAKE_CXX_COMPILER. Refer to the Darling docs \
for more details."
)

macro(clang_version_check compiler source_type clang_minimum_version)
    if (compiler STREQUAL "")
        message(FATAL_ERROR "Unable to find a compatible compiler")
    endif (compiler STREQUAL "")

    file(WRITE "${CMAKE_BINARY_DIR}/clang_major.${source_type}"  "#include <stdio.h>\n" 
    "#if !__clang__\n" "#error \"Not running on a clang compiler!\"\n" "#endif\n" "int main() { printf(\"%d\", __clang_major__); }")
    execute_process(COMMAND "${compiler}" "${CMAKE_BINARY_DIR}/clang_major.${source_type}" "-o" "clang_${source_type}_major"
    	RESULT_VARIABLE BUILD_CLANG_TEST_RESULT
    	OUTPUT_VARIABLE BUILD_CLANG_TEST_OUTPUT
		COMMAND_ECHO NONE
    )

    if (BUILD_CLANG_TEST_RESULT)
    	message(FATAL_ERROR
			"Failed to build ${CMAKE_BINARY_DIR}/clang_major.${source_type}\n"
			"This could indicate that `${compiler}` is either not a clang compiler, "
			"or the path does not exist. ${MANUALLY_SET_COMPILER_ERROR_MESSAGE}")
    endif (BUILD_CLANG_TEST_RESULT)

    execute_process(COMMAND "${CMAKE_BINARY_DIR}/clang_${source_type}_major"
    	RESULT_VARIABLE CLANG_MAJOR_VERSION_RESULT
    	OUTPUT_VARIABLE CLANG_MAJOR_VERSION_OUTPUT
    )

    if (CLANG_MAJOR_VERSION_RESULT)
        # This should normally never fail...
    	message(FATAL_ERROR "Failed to check clang major version")
    endif (CLANG_MAJOR_VERSION_RESULT)

    if ("${CLANG_MAJOR_VERSION_OUTPUT}" LESS ${clang_minimum_version})
    	message(FATAL_ERROR
    		"Your clang version (${CLANG_MAJOR_VERSION_OUTPUT}) is below the recommend supported version (${clang_minimum_version})\n"
			"${MANUALLY_SET_COMPILER_ERROR_MESSAGE}")
    endif ("${CLANG_MAJOR_VERSION_OUTPUT}" LESS ${clang_minimum_version})
endmacro()
