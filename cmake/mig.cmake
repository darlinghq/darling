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
        
        if (TARGET_ARM64)
            set(MIG_FLAGS "${MIG_FLAGS} -D__arm64__")
        endif (TARGET_ARM64)

        # message(STATUS "MIG flags: ${MIG_FLAGS}")
        separate_arguments(MIG_FLAGS)

        string(REPLACE ".defs" "" relativeName "${defFileName}")
        get_filename_component(bareName "${relativeName}" NAME)
        get_filename_component(dirName "${relativeName}" DIRECTORY)

        #if ((NOT BITS) OR (BITS EQUAL 64))
        #if(NOT arch)
        #	set(MIG_ARCH "x86-64")
        #else (NOT arch)
        #	set (MIG_ARCH "${arch}")
        #endif(NOT arch)
        if (NOT MIG_ARCH)
            if (TARGET_x86_64 OR TARGET_i386)
                set(MIG_ARCH "i386")
            elseif (TARGET_ARM64)
                set(MIG_ARCH "arm64")
            endif (TARGET_x86_64 OR TARGET_i386)
        endif (NOT MIG_ARCH)

	add_custom_command(OUTPUT
		${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_USER_SOURCE_SUFFIX}
		${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_USER_HEADER_SUFFIX}
		${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_SERVER_SOURCE_SUFFIX}
		${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_SERVER_HEADER_SUFFIX}
		${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_XTRACE_SUFFIX}
		COMMAND
			/bin/mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/${dirName} \;
			${MIG_EXECUTABLE}
			-arch ${MIG_ARCH}
			-user ${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_USER_SOURCE_SUFFIX}
			-header ${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_USER_HEADER_SUFFIX}
			-server ${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_SERVER_SOURCE_SUFFIX}
			-sheader ${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_SERVER_HEADER_SUFFIX}
			-xtracemig ${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_XTRACE_SUFFIX}
			${MIG_FLAGS}
			${CMAKE_CURRENT_SOURCE_DIR}/${defFileName}
		DEPENDS
			migexe migcom
	)

	if (NOT TARGET ${bareName}_xtrace_mig AND NOT MIG_NO_XTRACE)
		add_darling_library(${bareName}_xtrace_mig ${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_XTRACE_SUFFIX})
		target_compile_options(${bareName}_xtrace_mig PRIVATE
			"-I" "${CMAKE_SOURCE_DIR}/src/xtrace/include"
			"-Wno-extern-initializer")
		install(TARGETS ${bareName}_xtrace_mig DESTINATION "libexec/darling/usr/lib/darling/xtrace-mig/")
	endif (NOT TARGET ${bareName}_xtrace_mig AND NOT MIG_NO_XTRACE)
endfunction(mig)
