if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0011 NEW)
endif(COMMAND cmake_policy)

include(use_ld64)

FUNCTION(add_darling_executable exe)
	foreach(f IN LISTS ARGN)
		set(files ${files} ${f})
	endforeach(f)

	add_executable(${exe} ${files})
	set_property(TARGET ${exe} APPEND_STRING PROPERTY
		LINK_FLAGS " ${CMAKE_EXE_LINKER_FLAGS} -L${CMAKE_BINARY_DIR}/src/csu/CMakeFiles/csu.dir ")
	use_ld64(${exe})
	add_dependencies(${exe} csu)
ENDFUNCTION(add_darling_executable)

