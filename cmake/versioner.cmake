function(versioner project versions default_version)

	set(PROJECT ${project})
	string(TOUPPER ${project} UPROJECT)

	# /usr/bin/python is the versioner wrapper shared by the guest's Python
	# frameworks. With both frameworks installed, Python 3 is the default and
	# Python 2.7 remains selectable through the usual versioner mechanisms.
	if (project STREQUAL "python" AND DEFINED PYTHON3_VERSION)
		set(versions "2.7;${PYTHON3_VERSION}")
		set(default_version "${PYTHON3_VERSION}")
	endif()

	set(DEFAULTVERSION ${default_version})

	set(VERSIONS "")
	list(LENGTH versions NVERSIONS)

	foreach(version IN LISTS versions)
		set(VERSIONS "${VERSIONS} \"${version}\",")
	endforeach(version)

	configure_file(${CMAKE_SOURCE_DIR}/src/external/perl/versioner/versions.h.in ${CMAKE_CURRENT_BINARY_DIR}/versions.h)
	include_directories(${CMAKE_CURRENT_BINARY_DIR})

	add_definitions(-std=c99)
	add_darling_executable(${project} ${CMAKE_SOURCE_DIR}/src/external/perl/versioner/versioner.c)
	target_link_libraries(${project} CoreFoundation)

	install(TARGETS ${project} DESTINATION libexec/darling/usr/bin)
endfunction(versioner)
