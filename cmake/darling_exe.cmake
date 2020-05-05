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
		LINK_FLAGS " ${CMAKE_EXE_LINKER_FLAGS} -nostdlib ${CMAKE_BINARY_DIR}/src/csu/CMakeFiles/csu.dir/crt1.10.6.o ")

	if (TARGET_x86_64)
		target_compile_options(${exe} PRIVATE -arch x86_64)
	elseif (TARGET_i386)
		target_compile_options(${exe} PRIVATE -arch i386)
		set_property(TARGET ${exe} APPEND_STRING PROPERTY
			LINK_FLAGS " -arch i386")
	endif (TARGET_x86_64)

	use_ld64(${exe})
	target_link_libraries(${exe} system)
	add_dependencies(${exe} csu)

	if ((NOT NO_DSYM) AND (NOT ${exe}_NO_DSYM))
		dsym(${exe})
	endif ((NOT NO_DSYM) AND (NOT ${exe}_NO_DSYM))
ENDFUNCTION(add_darling_executable)

