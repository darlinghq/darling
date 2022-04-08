
function(dsym target)
	string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type)
	if (DSYMUTIL_EXE AND build_type MATCHES debug)

		add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${target}.dSYM" DEPENDS "${target}" COMMAND ${CMAKE_COMMAND} -E env
			"PATH=${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc:$ENV{PATH}"
			"${DSYMUTIL_EXE}" "-flat" "-o" "${target}.dSYM" "$<TARGET_FILE:${target}>")

		add_custom_target("${target}-dSYM" ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${target}.dSYM" getuuid lipo)

		install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${target}.dSYM" DESTINATION "${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Caches/dsym/files")
		install(DIRECTORY DESTINATION "${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Caches/dsym/uuid")

		install(CODE "execute_process(COMMAND \"${CMAKE_BINARY_DIR}/src/buildtools/getuuid\" \"${CMAKE_CURRENT_BINARY_DIR}/${target}.dSYM\" RESULT_VARIABLE getuuid_result OUTPUT_VARIABLE macho_uuid)

			if (NOT \${getuuid_result} EQUAL 0)
				message(WARNING \"Cannot determine UUID of ${target}\")
			else()
				# message(STATUS \"Installing dsym for ${target}\")
				foreach (uuid \${macho_uuid})
					# message(STATUS \"    UUID \${uuid}\")

					execute_process(COMMAND \"${CMAKE_COMMAND}\" -E create_symlink
						\"../files/${target}.dSYM\"
						\$ENV{DESTDIR}/${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Caches/dsym/uuid/\${uuid}.dSYM)
				endforeach (uuid)
			endif()
		")
	endif ()

endfunction(dsym)

function(FindDsymutil)
	# llvm-dsymutil-4.0 is not listed, because it's very buggy
	find_program(DSYMUTIL_EXE NAMES "llvm-dsymutil" "dsymutil" "dsymutil-7" "llvm-dsymutil-7.0" "llvm-dsymutil-6.0" "llvm-dsymutil-5.0" "llvm-dsymutil-3.9" "llvm-dsymutil-3.8" "llvm-dsymutil-3.7" "dsymutil-10 dsymutil-11 dsymutil-12 dsymutil-13 dsymutil-14 dsymutil-15 dsymutil-16")
	if (DSYMUTIL_EXE)
		message(STATUS "Found dsymutil: ${DSYMUTIL_EXE}")
	else (DSYMUTIL_EXE)
		message(WARNING "Cannot find llvm-dsymutil, will not produce debug information")
	endif (DSYMUTIL_EXE)
endfunction(FindDsymutil)
