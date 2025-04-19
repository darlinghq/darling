if(COMMAND cmake_policy)
	# cmake_policy(SET CMP0177 NEW)
endif(COMMAND cmake_policy)

include(darling_lib)
#include(CMakeParseArguments)

function(wrap_elf name elfname)
	add_custom_command(
		OUTPUT
			${CMAKE_CURRENT_BINARY_DIR}/${name}.c
		COMMAND
			${CMAKE_BINARY_DIR}/src/libelfloader/wrapgen/wrapgen
				${elfname} 
				${CMAKE_CURRENT_BINARY_DIR}/${name}.c
				${CMAKE_CURRENT_BINARY_DIR}/${name}_vars.h
		DEPENDS
			wrapgen
	)

	if (ARGC LESS 3)
		set(destination "usr/lib/native")
	else (ARGC LESS 3)
		set(destination ${ARGV2})
	endif (ARGC LESS 3)

	set(DYLIB_INSTALL_NAME "/${destination}/lib${name}.dylib")
	include_directories(${CMAKE_SOURCE_DIR}/src/startup/mldr/elfcalls)
	add_darling_library(${name} SHARED ${CMAKE_CURRENT_BINARY_DIR}/${name}.c)
	target_link_libraries(${name} PRIVATE system)
	make_fat(${name})
	install(TARGETS ${name} DESTINATION libexec/darling/${destination})
endfunction(wrap_elf)

