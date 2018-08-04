function(xcproj)
	set(options)
	set(oneValueArgs PROJECT)
	set(multiValueArgs EXECUTABLES LIBRARIES FRAMEWORKS)
	cmake_parse_arguments(xcproj "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	set(product_dir ${CMAKE_CURRENT_BINARY_DIR}/Build/Products/Release)
	
	foreach(executable ${xcproj_EXECUTABLES})
		set(output ${output} ${product_dir}/${executable})
	endforeach()

	foreach(library ${xcproj_LIBRARIES})
		set(output ${output} ${product_dir}/${library}.dylib)
	endforeach()

	foreach(framework ${xcproj_FRAMEWORKS})
		set(output ${output} ${product_dir}/${framework}.framework/${framework})
	endforeach()

	add_custom_command(
		OUTPUT ${output}
		COMMAND env RUNTIME_SPEC_PATH=${CMAKE_SOURCE_DIR}/src/external/xcbuild/Specifications/ DEVELOPER_DIR=${CMAKE_SOURCE_DIR}/Developer/ ${CMAKE_BINARY_DIR}/src/external/xcbuild/xcodebuild -project ${xcproj_PROJECT}
		DEPENDS xcbuild
	)
endfunction(xcproj)
