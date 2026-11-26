if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0011 NEW)
endif(COMMAND cmake_policy)

include(use_ld64)
include(architecture)

# darling_executable_archs(<out-var>): the slices executables are built for: the 64-bit ones, or the
# 32-bit one when no 64-bit slice is enabled. Objects an executable links (an object library, say)
# must be built for these slices.
function(darling_executable_archs out)
	if (DARLING_ARCHS_64BIT)
		set(${out} ${DARLING_ARCHS_64BIT} PARENT_SCOPE)
	else ()
		set(${out} ${DARLING_ARCHS_32BIT} PARENT_SCOPE)
	endif ()
endfunction()

FUNCTION(add_darling_executable exe)
	foreach(f IN LISTS ARGN)
		set(files ${files} ${f})
	endforeach(f)

	add_executable(${exe} ${files})
	set_property(TARGET ${exe} APPEND_STRING PROPERTY
		LINK_FLAGS " ${CMAKE_EXE_LINKER_FLAGS} -nostdlib ${CMAKE_BINARY_DIR}/src/external/csu/CMakeFiles/crt1.10.6.dir/start.S.o ")

	darling_executable_archs(exe_archs)
	foreach (arch IN LISTS exe_archs)
		target_compile_options(${exe} PRIVATE "SHELL:-arch ${arch}")
	endforeach ()
	darling_arch_flags(exe_arch_flags ${exe_archs})
	set_property(TARGET ${exe} APPEND_STRING PROPERTY LINK_FLAGS " ${exe_arch_flags}")
	list(LENGTH exe_archs exe_arch_count)
	if (exe_arch_count GREATER 1)
		# The driver joins the slices with lipo
		target_compile_options(${exe} PRIVATE "SHELL:-B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/")
		add_dependencies(${exe} lipo)
	endif ()

	use_ld64(${exe})
	target_link_libraries(${exe} system)
	add_dependencies(${exe} crt1.10.6)

	if ((NOT NO_DSYM) AND (NOT ${exe}_NO_DSYM))
		dsym(${exe})
	endif ((NOT NO_DSYM) AND (NOT ${exe}_NO_DSYM))
ENDFUNCTION(add_darling_executable)

