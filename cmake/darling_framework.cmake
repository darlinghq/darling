include(CMakeParseArguments)
include(darling_lib)
include(InstallSymlink)

function(add_framework name)
	cmake_parse_arguments(FRAMEWORK "CURRENT_VERSION;FAT" "VERSION;LINK_FLAGS" "SOURCES;DEPENDENCIES" ${ARGN})
	if (FRAMEWORK_CURRENT_VERSION)
		set(my_name "${name}")
	else (FRAMEWORK_CURRENT_VERSION)
		set(my_name "${name}_${FRAMEWORK_VERSION}")
	endif (FRAMEWORK_CURRENT_VERSION)

	set(DYLIB_INSTALL_NAME "/System/Library/Frameworks/${name}.framework/Versions/${FRAMEWORK_VERSION}/${name}")
	add_darling_library(${my_name} SHARED ${FRAMEWORK_SOURCES})

	set_target_properties(${my_name} PROPERTIES
				OUTPUT_NAME "${name}"
		        SUFFIX ""
				PREFIX "")

	if (FRAMEWORK_DEPENDENCIES)
		target_link_libraries(${my_name} PRIVATE ${FRAMEWORK_DEPENDENCIES})
	endif (FRAMEWORK_DEPENDENCIES)

	if (FRAMEWORK_FAT)
		make_fat(${my_name})
	endif (FRAMEWORK_FAT)

	if (FRAMEWORK_LINK_FLAGS)
		set_property(TARGET ${my_name} APPEND_STRING PROPERTY LINK_FLAGS " ${FRAMEWORK_LINK_FLAGS}")
	endif (FRAMEWORK_LINK_FLAGS)

	install(TARGETS ${my_name} DESTINATION "libexec/darling/System/Library/Frameworks/${name}.framework/Versions/${FRAMEWORK_VERSION}/")

	if (FRAMEWORK_CURRENT_VERSION)
		InstallSymlink(${FRAMEWORK_VERSION} "libexec/darling/System/Library/Frameworks/${name}.framework/Versions/Current")
		InstallSymlink("Versions/Current/${name}" "libexec/darling/System/Library/Frameworks/${name}.framework/${name}")
	endif (FRAMEWORK_CURRENT_VERSION)
endfunction(add_framework)


