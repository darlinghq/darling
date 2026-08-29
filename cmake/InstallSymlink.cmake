macro(InstallSymlink _filepath _sympath)
    cmake_parse_arguments(INSTALL_SYMLINK "EXCLUDE_FROM_ALL" "COMPONENT" "" ${ARGN})

    get_filename_component(_symname ${_sympath} NAME)
    get_filename_component(_installdir ${_sympath} PATH)

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

    install(CODE "
        if (\"\$ENV{DESTDIR}\" STREQUAL \"\")
            execute_process(COMMAND mkdir -p ${_installdir})
            execute_process(COMMAND ln -sf ${_filepath} ${_installdir}/${_symname})
        else ()
            execute_process(COMMAND mkdir -p \$ENV{DESTDIR}${_installdir})
            execute_process(COMMAND ln -sf ${_filepath} \$ENV{DESTDIR}${_installdir}/${_symname})
        endif ()
    " ${EXCLUDE_FROM_ALL_ARG} ${COMPONENT_ARG})
endmacro(InstallSymlink)
