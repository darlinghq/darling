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
            set(developer_sys_library_dir "System/iOSSupport/System/Library")
        else (SDK_IOSSUPPORT)
            set(developer_sys_library_dir "System/Library")
        endif (SDK_IOSSUPPORT)

        if (SDK_PRIVATE)
            set(developer_framework_dir "PrivateFrameworks")
            set(header_framework_include "framework-private-include")
        else (SDK_PRIVATE)
            set(developer_framework_dir "Frameworks")
            set(header_framework_include "framework-include")
        endif (SDK_PRIVATE)

        set(developer_platform "MacOSX.platform")
        set(developer_sdk "MacOSX.sdk")

        set(developer_sdk_path "Developer/Platforms/${developer_platform}/Developer/SDKs/${developer_sdk}")
        set(developer_framework_path "${DARLING_TOP_DIRECTORY}/${developer_sdk_path}/${developer_sys_library_dir}/${developer_framework_dir}/${name}.framework")
        set(header_framework_include_path "${DARLING_TOP_DIRECTORY}/${header_framework_include}/${name}")

        # Remove file from 'Developer' folder
        file(REMOVE_RECURSE ${developer_framework_path})
        # Also remove the the header folder from the framework/framework-private-include header
        file(REMOVE_RECURSE ${header_framework_include_path})

        message("Deleted SDK framework ${developer_sdk_path}")
    endif (REGENERATE_SDK)
endfunction(remove_sdk_framework)

function(get_path_preframework result)
    cmake_parse_arguments(SDK
        "PRIVATE;IOSSUPPORT"
        ""
        ""
        ${ARGN}
    )

    set(developer_platform "MacOSX.platform")
    set(developer_sdk "MacOSX.sdk")
    set(developer_sdk_path "Developer/Platforms/${developer_platform}/Developer/SDKs/${developer_sdk}")

    if (SDK_IOSSUPPORT)
        set(developer_sys_library_dir "System/iOSSupport/System/Library")
    else (SDK_IOSSUPPORT)
        set(developer_sys_library_dir "System/Library")
    endif (SDK_IOSSUPPORT)

    if (SDK_PRIVATE)
        set(developer_framework_dir "PrivateFrameworks")
    else (SDK_PRIVATE)
        set(developer_framework_dir "Frameworks")
    endif (SDK_PRIVATE)

    set("${result}" "${DARLING_TOP_DIRECTORY}/${developer_sdk_path}/${developer_sys_library_dir}/${developer_framework_dir}" PARENT_SCOPE)
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

function(internal_generate_developer_framework name path)
    cmake_parse_arguments(SDK
        ""
        "VERSION;HEADER"
        ""
        ${ARGN}
    )

    set(developer_framework_path "${path}/${name}.framework")
    set(sdk_path_exact_version "${developer_framework_path}/Versions/${SDK_VERSION}")
    file(MAKE_DIRECTORY ${sdk_path_exact_version})

    # message(developer_sdk_path="${developer_sdk_path}")
    # message(developer_framework_path="${developer_framework_path}")
    # message(sdk_path_exact_version="${sdk_path_exact_version}")

    file(RELATIVE_PATH sdk_relative_dir_include "${sdk_path_exact_version}" ${CMAKE_CURRENT_SOURCE_DIR}/${SDK_HEADER})
    # message(sdk_relative_dir_include="${sdk_relative_dir_include}")

    create_symlink("${SDK_VERSION}" "${developer_framework_path}/Versions/Current")
    create_symlink("Versions/${SDK_VERSION}/Headers" "${developer_framework_path}/Headers")
    create_symlink("${sdk_relative_dir_include}" "${sdk_path_exact_version}/Headers")

    if(EXISTS "${sdk_path_exact_version}/Frameworks")
        create_symlink("Versions/${SDK_VERSION}/Frameworks" "${developer_framework_path}/Frameworks")
    endif()

    message("Generated SDK framework ${developer_framework_path}")
endfunction(internal_generate_developer_framework)

function(internal_generate_framework_include name path)
    cmake_parse_arguments(SDK
        "PRIVATE"
        ""
        ""
        ${ARGN}
    )

    if (SDK_PRIVATE)
        set(header_framework_include "framework-private-include")
    else (SDK_PRIVATE)
        set(header_framework_include "framework-include")
    endif (SDK_PRIVATE)

    set(developer_headers_path "${path}/${name}.framework/Headers")
    set(header_framework_include_absolute_path "${DARLING_TOP_DIRECTORY}/${header_framework_include}")

    file(RELATIVE_PATH sdk_relative_dir_include "${header_framework_include_absolute_path}" "${developer_headers_path}")
    create_symlink("${sdk_relative_dir_include}" "${header_framework_include_absolute_path}/${name}")

    message("Created header symbolic-link ${header_framework_include_absolute_path}/${name}")
endfunction(internal_generate_framework_include)

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

        internal_generate_developer_framework(${name}
            "${sdk_path}"
            VERSION ${SDK_VERSION}
            HEADER ${SDK_HEADER}
        )

        internal_generate_framework_include(${name}
            "${sdk_path}"
            ${PRIVATE}
        )
    endif (REGENERATE_SDK)
endfunction(generate_sdk_framework)

function(generate_sdk_subframework name)
    cmake_parse_arguments(SDK
        "PRIVATE"
        "VERSION;HEADER;BASE_PATH"
        ""
        ${ARGN}
    )

    if (SDK_PRIVATE)
        set(PRIVATE "PRIVATE")
    else (SDK_PRIVATE)
        set(PRIVATE "")
    endif (SDK_PRIVATE)

    internal_generate_developer_framework(${name}
        "${SDK_BASE_PATH}"
        VERSION ${SDK_VERSION}
        HEADER ${SDK_HEADER}
    )

    internal_generate_framework_include(${name}
        "${SDK_BASE_PATH}"
        ${PRIVATE}
    )
endfunction(generate_sdk_subframework)