#
# used to create a symbolic link at configuration time
#
function(create_symlink target output_path)
	if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14.0")
		# use CREATE_LINK if we can
		file(CREATE_LINK "${target}" "${output_path}" SYMBOLIC)
	else()
		# fallback to `ln` otherwise
		execute_process(COMMAND ln -sfn "${target}" "${output_path}")
	endif()
endfunction()
