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
		DEPENDS
			wrapgen
	)

	if (ARGC LESS 3)
		set(destination "/usr/lib/native")
	else (ARGC LESS 3)
		set(destination ${ARGV2})
	endif (ARGC LESS 3)

	set(DYLIB_INSTALL_NAME "${destination}/lib${name}.dylib")
	include_directories(${CMAKE_SOURCE_DIR}/src/libelfloader/native)
	add_darling_library(${name} SHARED ${CMAKE_CURRENT_BINARY_DIR}/${name}.c)
	target_link_libraries(${name} PRIVATE system elfloader)
	make_fat(${name})
	install(TARGETS ${name} DESTINATION libexec/darling/${destination})
endfunction(wrap_elf)

