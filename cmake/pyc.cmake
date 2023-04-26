include(CMakeParseArguments)

function(pyc target_name)
	cmake_parse_arguments(PYC "" "DESTINATION" "SOURCES" ${ARGN})
	set(generated_files "")

if (COMPILE_PY2_BYTECODE)
	foreach(pyfile ${PYC_SOURCES})
		STRING(REGEX REPLACE "^${CMAKE_CURRENT_SOURCE_DIR}" "" pyfile_rel ${pyfile})
		
		get_filename_component(bareName "${pyfile_rel}" NAME)
		get_filename_component(dirName "${pyfile_rel}" DIRECTORY)

		# message(STATUS "Process ${pyfile}, dirName ${dirName}, bareName ${bareName}")
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
endfunction(pyc)

