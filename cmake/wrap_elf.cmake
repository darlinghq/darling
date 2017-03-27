include(darling_lib)
#include(CMakeParseArguments)

function(wrap_elf name elfname)
	add_custom_command(
		OUTPUT
			${CMAKE_CURRENT_BINARY_DIR}/${name}.c
		COMMAND
			${CMAKE_BINARY_DIR}/src/dyld/wrapgen
				${elfname} 
				${CMAKE_CURRENT_BINARY_DIR}/${name}.c
		DEPENDS
			wrapgen
	)

	set(DYLIB_INSTALL_NAME "/usr/lib/native/lib${name}.dylib")
	include_directories(${CMAKE_SOURCE_DIR}/src/dyld)
	add_darling_library(${name} SHARED ${CMAKE_CURRENT_BINARY_DIR}/${name}.c)
	target_link_libraries(${name} PRIVATE system)
	make_fat(${name})
	install(TARGETS ${name} DESTINATION libexec/darling/usr/lib/native)
endfunction(wrap_elf)

