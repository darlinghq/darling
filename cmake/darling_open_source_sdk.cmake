include(create_symlink)

function(remove_sdk_framework name)
    cmake_parse_arguments(SDK
        "PRIVATE;IOSSUPPORT"
        ""
        ""
        ${ARGN}
    )

    if (REGENERATE_SDK)
        if (SDK_IOSSUPPORT)
            set(sdk_dir_sys_library "System/iOSSupport/System/Library")
        else (SDK_IOSSUPPORT)
            set(sdk_dir_sys_library "System/Library")
        endif (SDK_IOSSUPPORT)

        if (SDK_PRIVATE)
            set(sdk_dir_framework "PrivateFrameworks")
        else (SDK_PRIVATE)
            set(sdk_dir_framework "Frameworks")
        endif (SDK_PRIVATE)

        set(sdk_platform "MacOSX.platform")
        set(sdk_sdk "MacOSX.sdk")
        set(sdk_path_sdk "${DARLING_TOP_DIRECTORY}/Developer/Platforms/${sdk_platform}/Developer/SDKs/${sdk_sdk}")

        set(sdk_path_framework "${sdk_path_sdk}/${sdk_dir_sys_library}/${sdk_dir_framework}/${name}.framework")
        file(REMOVE_RECURSE ${sdk_path_framework})
        message("Deleted SDK framework ${sdk_path_sdk}")
    endif (REGENERATE_SDK)
endfunction(remove_sdk_framework)

function(get_path_preframework result)
    cmake_parse_arguments(SDK
        "PRIVATE;IOSSUPPORT"
        ""
        ""
        ${ARGN}
    )

    set(sdk_platform "MacOSX.platform")
    set(sdk_sdk "MacOSX.sdk")
    set(sdk_path_sdk "${DARLING_TOP_DIRECTORY}/Developer/Platforms/${sdk_platform}/Developer/SDKs/${sdk_sdk}")

    if (SDK_IOSSUPPORT)
        set(sdk_dir_sys_library "System/iOSSupport/System/Library")
    else (SDK_IOSSUPPORT)
        set(sdk_dir_sys_library "System/Library")
    endif (SDK_IOSSUPPORT)

    if (SDK_PRIVATE)
        set(sdk_dir_framework "PrivateFrameworks")
    else (SDK_PRIVATE)
        set(sdk_dir_framework "Frameworks")
    endif (SDK_PRIVATE)

    set("${result}" "${sdk_path_sdk}/${sdk_dir_sys_library}/${sdk_dir_framework}" PARENT_SCOPE)
endfunction(get_path_preframework)

function(append_path_sdk_subframework input_path output_path name)
    cmake_parse_arguments(SDK
        ""
        "VERSION"
        ""
        ${ARGN}
    )

    set("${output_path}" "${input_path}/${name}.framework/Versions/${SDK_VERSION}/Frameworks" PARENT_SCOPE)
endfunction(append_path_sdk_subframework)

function(internal_generate_sdk_framework name path)
    cmake_parse_arguments(SDK
        ""
        "VERSION;HEADER"
        ""
        ${ARGN}
    )

    set(sdk_path_framework "${path}/${name}.framework")
    set(sdk_path_exact_version "${sdk_path_framework}/Versions/${SDK_VERSION}")
    file(MAKE_DIRECTORY ${sdk_path_exact_version})

    # message(sdk_path_sdk="${sdk_path_sdk}")
    # message(sdk_path_framework="${sdk_path_framework}")
    # message(sdk_path_exact_version="${sdk_path_exact_version}")

    file(RELATIVE_PATH sdk_relative_dir_include "${sdk_path_exact_version}" ${CMAKE_CURRENT_SOURCE_DIR}/${SDK_HEADER})
    # message(sdk_relative_dir_include="${sdk_relative_dir_include}")

    create_symlink("${SDK_VERSION}" "${sdk_path_framework}/Versions/Current")
    create_symlink("Versions/${SDK_VERSION}/Headers" "${sdk_path_framework}/Headers")
    create_symlink("${sdk_relative_dir_include}" "${sdk_path_exact_version}/Headers")

    if(EXISTS "${sdk_path_exact_version}/Frameworks")
        create_symlink("Versions/${SDK_VERSION}/Frameworks" "${sdk_path_framework}/Frameworks")
    endif()

    message("Generated SDK framework ${sdk_path_framework}")
endfunction(internal_generate_sdk_framework)

function(generate_sdk_framework name)
    cmake_parse_arguments(SDK
        "PRIVATE;IOSSUPPORT"
        "VERSION;HEADER"
        ""
        ${ARGN}
    )

    if (REGENERATE_SDK)
        if (SDK_IOSSUPPORT)
            set(IOSSUPPORT "IOSSUPPORT")
        else (SDK_IOSSUPPORT)
            set(IOSSUPPORT "")
        endif (SDK_IOSSUPPORT)

        if (SDK_PRIVATE)
            set(PRIVATE "PRIVATE")
        else (SDK_PRIVATE)
            set(PRIVATE "")
        endif (SDK_PRIVATE)

        get_path_preframework(sdk_path
            ${PRIVATE}
            ${IOSSUPPORT}
        )

        internal_generate_sdk_framework(${name}
            "${sdk_path}"
            VERSION ${SDK_VERSION}
            HEADER ${SDK_HEADER}
        )
    endif (REGENERATE_SDK)
endfunction(generate_sdk_framework)

function(generate_sdk_subframework name)
    cmake_parse_arguments(SDK
        ""
        "VERSION;HEADER;BASE_PATH"
        ""
        ${ARGN}
    )

    internal_generate_sdk_framework(${name}
        "${SDK_BASE_PATH}"
        VERSION ${SDK_VERSION}
        HEADER ${SDK_HEADER}
    )
endfunction(generate_sdk_subframework)