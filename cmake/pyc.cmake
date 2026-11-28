include(CMakeParseArguments)

function(pyc target_name)
	cmake_parse_arguments(PYC "" "DESTINATION;PYTHON_VERSION;ROOT" "SOURCES" ${ARGN})

	if (NOT PYC_PYTHON_VERSION)
		set(PYC_PYTHON_VERSION "2.7")
	endif ()

	if (NOT PYC_ROOT)
		set(PYC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
	endif ()

	set(generated_files "")

	if (PYC_PYTHON_VERSION STREQUAL "2.7")
		if (COMPILE_PY2_BYTECODE)
			foreach(pyfile ${PYC_SOURCES})
				STRING(REGEX REPLACE "^${PYC_ROOT}" "" pyfile_rel ${pyfile})
				STRING(REGEX REPLACE "^/" "" pyfile_rel ${pyfile_rel})

				get_filename_component(bareName "${pyfile_rel}" NAME)
				get_filename_component(dirName "${pyfile_rel}" DIRECTORY)

				file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${dirName}")

				set(output_name "${CMAKE_CURRENT_BINARY_DIR}/${dirName}/${bareName}c")
				add_custom_command(OUTPUT "${output_name}"
					COMMAND "${CMAKE_SOURCE_DIR}/tools/pyc" "${pyfile}" "${CMAKE_CURRENT_BINARY_DIR}/${dirName}/${bareName}c"
					WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

				list(APPEND generated_files "${output_name}")
				install(FILES "${output_name}" DESTINATION "${PYC_DESTINATION}")
			endforeach(pyfile)

			add_custom_target("${target_name}" ALL DEPENDS ${generated_files})
		endif (COMPILE_PY2_BYTECODE)
	elseif (PYC_PYTHON_VERSION STREQUAL "3.13")
		if (COMPILE_PY3_BYTECODE)
			# A generic host python3 may be an older release, but the bytecode tag
			# below has to match the guest interpreter exactly. The top-level
			# configure step normally provides the checked interpreter.
			if (NOT PYTHON3_BYTECODE_EXECUTABLE)
				find_program(PYTHON3_BYTECODE_EXECUTABLE NAMES python3.13 python3)
			endif ()
			if (NOT PYTHON3_BYTECODE_EXECUTABLE)
				message(FATAL_ERROR "Pre-compiling Python 3.13 bytecode requires a host Python 3.13 interpreter")
			endif ()
			execute_process(
				COMMAND "${PYTHON3_BYTECODE_EXECUTABLE}" -c "import sys; print('%d.%d' % sys.version_info[:2])"
				OUTPUT_VARIABLE PYTHON3_BYTECODE_VERSION
				OUTPUT_STRIP_TRAILING_WHITESPACE
				RESULT_VARIABLE PYTHON3_BYTECODE_VERSION_RESULT
			)
			if (NOT PYTHON3_BYTECODE_VERSION_RESULT EQUAL 0 OR NOT PYTHON3_BYTECODE_VERSION STREQUAL "3.13")
				message(FATAL_ERROR "Pre-compiling Python 3.13 bytecode requires a host Python 3.13 interpreter, found '${PYTHON3_BYTECODE_VERSION}' at ${PYTHON3_BYTECODE_EXECUTABLE}")
			endif ()

			foreach(pyfile ${PYC_SOURCES})
				STRING(REGEX REPLACE "^${PYC_ROOT}" "" pyfile_rel ${pyfile})
				STRING(REGEX REPLACE "^/" "" pyfile_rel ${pyfile_rel})

				get_filename_component(bareName "${pyfile_rel}" NAME_WE)
				get_filename_component(dirName "${pyfile_rel}" DIRECTORY)

				set(cache_dir "${CMAKE_CURRENT_BINARY_DIR}/pyc3/${dirName}/__pycache__")
				file(MAKE_DIRECTORY "${cache_dir}")

				set(output_name "${cache_dir}/${bareName}.cpython-313.pyc")

				# pyc files embed the source path used for tracebacks. Use the
				# guest path rather than the host build path.
				set(guest_dir "${PYC_DESTINATION}")
				string(REGEX REPLACE "^libexec/darling" "" guest_dir "${guest_dir}")
				if (dirName)
					set(guest_file "${guest_dir}/${dirName}/${bareName}.py")
					set(cache_destination "${PYC_DESTINATION}/${dirName}/__pycache__")
				else ()
					set(guest_file "${guest_dir}/${bareName}.py")
					set(cache_destination "${PYC_DESTINATION}/__pycache__")
				endif ()

				add_custom_command(OUTPUT "${output_name}"
					COMMAND "${PYTHON3_BYTECODE_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/pyc3" "${pyfile}" "${output_name}" "${guest_file}"
					WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
					VERBATIM)

				list(APPEND generated_files "${output_name}")
				install(FILES "${output_name}" DESTINATION "${cache_destination}")
			endforeach(pyfile)

			add_custom_target("${target_name}" ALL DEPENDS ${generated_files})
		endif (COMPILE_PY3_BYTECODE)
	else ()
		message(FATAL_ERROR "Unknown Python version passed to pyc(): ${PYC_PYTHON_VERSION}")
	endif ()
endfunction(pyc)
