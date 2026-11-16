if(COMMAND cmake_policy)
	# cmake_policy(SET CMP0177 NEW)
endif(COMMAND cmake_policy)

include(darling_lib)
#include(CMakeParseArguments)

# wrap_elf(name elfname [destination] [OPTIONAL <availability-function>])
#
# With OPTIONAL the wrapper does not abort the process at load time when the ELF
# library is missing on the host. It exports "int <availability-function>(void)",
# which returns non-zero if the library could be loaded; callers must check it
# before using any function of the library.
function(wrap_elf name elfname)
	set(destination "usr/lib/native")
	set(availability "")
	set(extra ${ARGN})
	if (extra)
		list(GET extra 0 first)
		if (NOT first STREQUAL "OPTIONAL")
			set(destination ${first})
			list(REMOVE_AT extra 0)
		endif ()
	endif ()
	if (extra)
		list(GET extra 0 keyword)
		if (NOT keyword STREQUAL "OPTIONAL")
			message(FATAL_ERROR "wrap_elf: expected OPTIONAL <availability-function>")
		endif ()
		list(GET extra 1 availability)
	endif ()

	set(_wrap_elfname "${elfname}")
	if (NOT IS_ABSOLUTE "${_wrap_elfname}" AND NOT EXISTS "${_wrap_elfname}")
		get_filename_component(_wrap_elf_base "${_wrap_elfname}" NAME_WE)
		string(REGEX REPLACE "^lib" "" _wrap_elf_libname "${_wrap_elf_base}")
		find_library(_WRAP_ELF_${name}_PATH NAMES ${_wrap_elf_libname} ${_wrap_elf_base} ${_wrap_elfname} NAMES_PER_DIR)
		if (_WRAP_ELF_${name}_PATH)
			set(_wrap_elfname "${_WRAP_ELF_${name}_PATH}")
		endif ()
	endif ()

	add_custom_command(
		OUTPUT
			${CMAKE_CURRENT_BINARY_DIR}/${name}.c
		COMMAND
			${CMAKE_BINARY_DIR}/src/libelfloader/wrapgen/wrapgen
				${_wrap_elfname}
				${CMAKE_CURRENT_BINARY_DIR}/${name}.c
				${CMAKE_CURRENT_BINARY_DIR}/${name}_vars.h
				${availability}
		DEPENDS
			wrapgen
	)

	set(DYLIB_INSTALL_NAME "/${destination}/lib${name}.dylib")
	include_directories(${CMAKE_SOURCE_DIR}/src/startup/mldr/elfcalls)
	add_darling_library(${name} SHARED ${CMAKE_CURRENT_BINARY_DIR}/${name}.c)
	target_link_libraries(${name} PRIVATE system)
	make_fat(${name})
	install(TARGETS ${name} DESTINATION libexec/darling/${destination})
endfunction(wrap_elf)

