include(create_symlink)

function(remove_sdk_framework name)
    cmake_parse_arguments(SDK
        "PRIVATE;IOSSUPPORT"
        "PARENT_DIR"
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

        if (SDK_PARENT_DIR)
            set(developer_framework_path "${DARLING_TOP_DIRECTORY}/${developer_sdk_path}/${SDK_PARENT_DIR}/${name}.framework")
        endif()

        set(header_framework_include_path "${DARLING_TOP_DIRECTORY}/${header_framework_include}/${name}")

        # Remove file from 'Developer' folder
        file(REMOVE_RECURSE ${developer_framework_path})
        # Also remove the the header folder from the framework/framework-private-include header
        file(REMOVE_RECURSE ${header_framework_include_path})

        message("Deleted SDK framework ${developer_framework_path}")
    endif (REGENERATE_SDK)
endfunction(remove_sdk_framework)

function(get_path_preframework result)
    cmake_parse_arguments(SDK
        "PRIVATE;IOSSUPPORT"
        "PARENT_DIR"
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

    set(developer_framework_path "${DARLING_TOP_DIRECTORY}/${developer_sdk_path}/${developer_sys_library_dir}/${developer_framework_dir}")

    if (SDK_PARENT_DIR)
        set(developer_framework_path "${DARLING_TOP_DIRECTORY}/${developer_sdk_path}/${SDK_PARENT_DIR}")
    endif()

    set("${result}" "${developer_framework_path}" PARENT_SCOPE)
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
        "VERSION;HEADER;PARENT_DIR"
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
        "VERSION;HEADER;PARENT_DIR"
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
            PARENT_DIR "${SDK_PARENT_DIR}"
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

    if (REGENERATE_SDK)
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
    endif (REGENERATE_SDK)
endfunction(generate_sdk_subframework)

# The header surface is not copied: framework-include/<F> is a symlink to
# <F>.framework/Headers, which is itself a symlink to the owning submodule's
# include/<F> directory. A clone therefore only has a usable SDK once every
# submodule working tree exists, and nothing used to say so. The build then
# failed much later, per translation unit, as missing-declaration errors in
# whichever component happened to include the absent header.
#
# This checks that each framework's header directory resolves, which covers an
# uninitialised submodule. It deliberately does not walk the headers inside: the
# submodules also carry symlinks of their own, and darling-corefoundation's
# point into a nested submodule, so some headers can be absent while the
# directory resolves. The diagnostic therefore names the recursive form.
function(verify_sdk_header_include)
    set(unresolved "")
    foreach (include_dir IN ITEMS framework-include framework-private-include)
        file(GLOB frameworks "${DARLING_TOP_DIRECTORY}/${include_dir}/*")
        foreach (framework IN LISTS frameworks)
            if (NOT IS_DIRECTORY "${framework}")
                list(APPEND unresolved "${framework}")
            endif()
        endforeach()
    endforeach()

    if (unresolved)
        list(LENGTH unresolved unresolved_count)
        list(SORT unresolved)
        list(JOIN unresolved "\n  " unresolved_list)
        message(FATAL_ERROR
            "${unresolved_count} of the SDK's header directories do not resolve, so the\n"
            "headers they expose are missing and the build would fail later with\n"
            "unrelated-looking errors. They are symlinks into the submodules' include\n"
            "directories, so the submodule working trees must be populated:\n"
            "    git submodule update --init --recursive\n"
            "Unresolved:\n  ${unresolved_list}")
    endif()
endfunction(verify_sdk_header_include)
