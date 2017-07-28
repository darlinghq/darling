function(mig defFileName)
        set(MIG_EXECUTABLE "${CMAKE_BINARY_DIR}/mig")

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
        
        #if ((NOT BITS) OR (BITS EQUAL 64))
        #if(NOT arch)
        #	set(MIG_ARCH "x86-64")
        #else (NOT arch)
        #	set (MIG_ARCH "${arch}")
        #endif(NOT arch)
        if (NOT MIG_ARCH)
            set(MIG_ARCH "x86-64")
        endif (NOT MIG_ARCH)

	add_custom_command(OUTPUT
		${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_USER_SOURCE_SUFFIX}
		${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_USER_HEADER_SUFFIX}
		${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_SERVER_SOURCE_SUFFIX}
		${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_SERVER_HEADER_SUFFIX}
		COMMAND
			/bin/mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/{$dirName} \;
			${MIG_EXECUTABLE}
			-arch ${MIG_ARCH}
			-user ${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_USER_SOURCE_SUFFIX}
			-header ${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_USER_HEADER_SUFFIX}
			-server ${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_SERVER_SOURCE_SUFFIX}
			-sheader ${CMAKE_CURRENT_BINARY_DIR}/${relativeName}${MIG_SERVER_HEADER_SUFFIX}
			${MIG_FLAGS}
			${CMAKE_CURRENT_SOURCE_DIR}/${defFileName}
		DEPENDS
			migcom
	)
endfunction(mig)
