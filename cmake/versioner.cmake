function(versioner project versions default_version)

	set(PROJECT ${project})
	string(TOUPPER ${project} UPROJECT)
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
