# Create developer dir symlinks

add_custom_target(xcproj_symlinks
	DEPENDS Foundation
	COMMAND env BUILD=${CMAKE_BUID_DIR} ${CMAKE_SOURCE_DIR}/Developer/symlinks.sh
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/Developer
)

function(xcproj)
	set(options)
	set(oneValueArgs PROJECT)
	set(multiValueArgs EXECUTABLES LIBRARIES FRAMEWORKS)
	cmake_parse_arguments(xcproj "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	set(product_dir ${CMAKE_CURRENT_BINARY_DIR}/Build/Products/Release)
	
	foreach(executable IN LISTS xcproj_EXECUTABLES)
		set(output ${output} ${product_dir}/${executable})
	endforeach()

	foreach(library IN LISTS xcproj_LIBRARIES)
		set(output ${output} ${product_dir}/${library}.dylib)
	endforeach()

	foreach(framework IN LISTS xcproj_FRAMEWORKS)
		set(output ${output} ${product_dir}/${framework}.framework/${framework})
	endforeach()

	add_custom_command(
		OUTPUT ${output}
		COMMAND env RUNTIME_SPEC_PATH=${CMAKE_SOURCE_DIR}/src/external/xcbuild/Specifications DEVELOPER_DIR=${CMAKE_SOURCE_DIR}/Developer ${CMAKE_BINARY_DIR}/src/external/xcbuild/Libraries/xcdriver/xcodebuild -project ${CMAKE_CURRENT_SOURCE_DIR}/${xcproj_PROJECT} -derivedDataPath ${CMAKE_CURRENT_BINARY_DIR}
		DEPENDS xcbuild xcproj_symlinks
	)

	foreach(framework IN LISTS xcproj_FRAMEWORKS)
		add_custom_target(${framework} ALL
			DEPENDS ${product_dir}/${framework}.framework/${framework}
		)
	endforeach()
endfunction(xcproj)
