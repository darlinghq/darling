include(create_symlink)

function(generate_sdk_header name)
    cmake_parse_arguments(SDK
        "PRIVATE;IOSSUPPORT"
        "VERSION;HEADER"
        ""
        ${ARGN}
    )

    if (REGENERATE_SDK)
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

        set(sdk_path_framework "${sdk_path_sdk}/${sdk_dir_sys_library}/${sdk_dir_framework}/${name}.framework")
        file(REMOVE_RECURSE ${sdk_path_framework})

        set(sdk_path_exact_version "${sdk_path_framework}/Versions/${SDK_VERSION}")
        file(MAKE_DIRECTORY ${sdk_path_exact_version})

        message(sdk_path_sdk="${sdk_path_sdk}")
        message(sdk_path_framework="${sdk_path_framework}")
        message(sdk_path_exact_version="${sdk_path_exact_version}")

        file(RELATIVE_PATH sdk_relative_dir_include "${sdk_path_exact_version}" ${CMAKE_CURRENT_SOURCE_DIR}/${SDK_HEADER})
        message(sdk_relative_dir_include="${sdk_relative_dir_include}")

        create_symlink("${SDK_VERSION}" "${sdk_path_framework}/Versions/Current")
        create_symlink("Versions/${SDK_VERSION}/Headers" "${sdk_path_framework}/Headers")
        create_symlink("${sdk_relative_dir_include}" "${sdk_path_exact_version}/Headers")
    endif (REGENERATE_SDK)
endfunction(generate_sdk_header)