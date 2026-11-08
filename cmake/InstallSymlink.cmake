# InstallSymlink(<target> <link> [COMPONENT <component>] [EXCLUDE_FROM_ALL])
#
# Installs a symbolic link at <link> that points to <target>. The link is
# created at install time, under $DESTDIR if it is set, together with its
# parent directories; an existing link there is replaced. A relative <link> is
# taken relative to CMAKE_INSTALL_PREFIX. <target> is stored as given, so a
# relative target stays relative to the link's directory. Failing to create the
# directory or the link fails the install.
#
#   target: what the link points to (absolute, or relative to the link)
#   link: path of the installed link (absolute, or relative to the prefix)

macro(InstallSymlink _filepath _sympath)
    cmake_parse_arguments(INSTALL_SYMLINK "EXCLUDE_FROM_ALL" "COMPONENT" "" ${ARGN})

    get_filename_component(_symname "${_sympath}" NAME)
    get_filename_component(_installdir "${_sympath}" PATH)

    if (NOT IS_ABSOLUTE "${_installdir}")
        set(_installdir "${CMAKE_INSTALL_PREFIX}/${_installdir}")
    endif()

    if (INSTALL_SYMLINK_EXCLUDE_FROM_ALL)
        set(EXCLUDE_FROM_ALL_ARG "EXCLUDE_FROM_ALL")
    else()
        set(EXCLUDE_FROM_ALL_ARG "")
    endif()

    if (DEFINED INSTALL_SYMLINK_COMPONENT)
        set(COMPONENT_ARG COMPONENT "${INSTALL_SYMLINK_COMPONENT}")
    else()
        set(COMPONENT_ARG "")
    endif()

    # Expand configure-time paths while preserving DESTDIR for installation.
    # CMake commands keep paths containing spaces intact and let us propagate
    # errors instead of producing a successful but incomplete package.
    set(_symlink_target "${_filepath}")
    set(_symlink_code [=[
        set(_symlink_dir "$ENV{DESTDIR}@_installdir@")
        execute_process(
            COMMAND "@CMAKE_COMMAND@" -E make_directory "${_symlink_dir}"
            RESULT_VARIABLE _symlink_result)
        if(NOT _symlink_result STREQUAL "0")
            message(FATAL_ERROR "Cannot create symlink directory ${_symlink_dir}: ${_symlink_result}")
        endif()
        execute_process(
            COMMAND "@CMAKE_COMMAND@" -E create_symlink "@_symlink_target@" "${_symlink_dir}/@_symname@"
            RESULT_VARIABLE _symlink_result)
        if(NOT _symlink_result STREQUAL "0")
            message(FATAL_ERROR "Cannot install symlink ${_symlink_dir}/@_symname@: ${_symlink_result}")
        endif()
    ]=])
    string(CONFIGURE "${_symlink_code}" _symlink_code @ONLY)
    install(CODE "${_symlink_code}" ${EXCLUDE_FROM_ALL_ARG} ${COMPONENT_ARG})
endmacro(InstallSymlink)
