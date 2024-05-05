function(mig defFileName)
        set(MIG_EXECUTABLE "${CMAKE_BINARY_DIR}/build-mig")

        if (NOT MIG_USER_SOURCE_SUFFIX)
                set (MIG_USER_SOURCE_SUFFIX "User.c")
        endif (NOT MIG_USER_SOURCE_SUFFIX)
        if (NOT MIG_USER_HEADER_SUFFIX)
                set (MIG_USER_HEADER_SUFFIX ".h")
        endif (NOT MIG_USER_HEADER_SUFFIX)
        if (NOT MIG_SERVER_SOURCE_SUFFIX)
                set (MIG_SERVER_SOURCE_SUFFIX "Server.c")
        endif (NOT MIG_SERVER_SOURCE_SUFFIX)
        if (NOT MIG_SERVER_HEADER_SUFFIX)
                set (MIG_SERVER_HEADER_SUFFIX "Server.h")
        endif (NOT MIG_SERVER_HEADER_SUFFIX)
        if (NOT MIG_XTRACE_SUFFIX)
                set (MIG_XTRACE_SUFFIX "XtraceMig.c")
        endif (NOT MIG_XTRACE_SUFFIX)

        get_directory_property(DirDefs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMPILE_DEFINITIONS)
        get_directory_property(InclDirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} INCLUDE_DIRECTORIES)

        set(MIG_FLAGS "")
        foreach (def ${DirDefs})
                set(MIG_FLAGS "${MIG_FLAGS} -D${def}")
        endforeach()
        foreach (dir ${InclDirs})
                set(MIG_FLAGS "${MIG_FLAGS} -I${dir}")
        endforeach()

        # message(STATUS "MIG flags: ${MIG_FLAGS}")
        separate_arguments(MIG_FLAGS)

        string(REPLACE ".defs" "" relativeName "${defFileName}")
        get_filename_component(bareName "${relativeName}" NAME)
        get_filename_component(dirName "${relativeName}" DIRECTORY)

	if (NOT MIG_MULTIARCH)
		set(MIG_MULTIARCH_NO_SUFFIX 1)
		if (NOT MIG_ARCH)
			set(MIG_MULTIARCH "i386")
		else()
			set(MIG_MULTIARCH "${MIG_ARCH}")
		endif()
	endif()

	# TODO: Add support for multi-triplet targets
	set(MIG_TARGET_TRIPLET_PRIMARY "${APPLE_TARGET_TRIPLET_PRIMARY}")

	foreach(MIG_ARCH ${MIG_MULTIARCH})
		if (MIG_MULTIARCH_NO_SUFFIX)
			set(MIG_ARCH_SUFFIX "")
		else()
			set(MIG_ARCH_SUFFIX "-${MIG_ARCH}-")
		endif()

		add_custom_command(OUTPUT
			${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_ARCH_SUFFIX}${MIG_USER_SOURCE_SUFFIX}
			${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_ARCH_SUFFIX}${MIG_USER_HEADER_SUFFIX}
			${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_ARCH_SUFFIX}${MIG_SERVER_SOURCE_SUFFIX}
			${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_ARCH_SUFFIX}${MIG_SERVER_HEADER_SUFFIX}
			${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_ARCH_SUFFIX}${MIG_XTRACE_SUFFIX}
			COMMAND
				/bin/mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/${dirName} \;
				${MIG_EXECUTABLE}
				-arch ${MIG_ARCH}
				-target ${MIG_TARGET_TRIPLET_PRIMARY}
				-user ${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_ARCH_SUFFIX}${MIG_USER_SOURCE_SUFFIX}
				-header ${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_ARCH_SUFFIX}${MIG_USER_HEADER_SUFFIX}
				-server ${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_ARCH_SUFFIX}${MIG_SERVER_SOURCE_SUFFIX}
				-sheader ${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_ARCH_SUFFIX}${MIG_SERVER_HEADER_SUFFIX}
				-xtracemig ${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_ARCH_SUFFIX}${MIG_XTRACE_SUFFIX}
				${MIG_FLAGS}
				${CMAKE_CURRENT_SOURCE_DIR}/${defFileName} \;
				# this is so that the xtrace file is always produced so that the command is not constantly re-run
				# for MIG definitions that produce no xtrace files
				touch ${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_ARCH_SUFFIX}${MIG_XTRACE_SUFFIX}
			DEPENDS
				migexe migcom
		)

		if (NOT TARGET ${bareName}_xtrace_mig AND NOT MIG_NO_XTRACE)
			add_darling_library(${bareName}_xtrace_mig ${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_ARCH_SUFFIX}${MIG_XTRACE_SUFFIX})
			target_compile_options(${bareName}_xtrace_mig PRIVATE
				"-I" "${CMAKE_SOURCE_DIR}/src/xtrace/include"
				"-Wno-extern-initializer")
			install(TARGETS ${bareName}_xtrace_mig DESTINATION "libexec/darling/usr/lib/darling/xtrace-mig/")
		endif (NOT TARGET ${bareName}_xtrace_mig AND NOT MIG_NO_XTRACE)
	endforeach()
endfunction(mig)
