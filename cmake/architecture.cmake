# The architectures (slices) Darling's Mach-O binaries are built for.
#
# DARLING_ARCHS, set when configuring (for example -DDARLING_ARCHS="i386;x86_64;arm64"), lists every
# enabled slice. Any non-empty combination of the architectures below may be given; the list is put
# into the canonical order i386, x86_64, arm64, so the order on the command line does not matter.
# Components loop over the list (or a subset from darling_archs()) once, with i386 as one entry
# rather than a separate 32-bit path, and ask the darling_arch_* functions for what differs per slice.
#
# Every architecture Darling knows how to build, in the canonical order. A new slice has to be
# described here and in the darling_arch_* functions before any component can build it.
set(DARLING_SUPPORTED_ARCHS i386 x86_64 arm64)

# darling_arch_triple(<arch> <out-var>): the target triple of the slice.
function(darling_arch_triple arch out)
    if (arch STREQUAL "i386")
        set(${out} "i386-apple-darwin20" PARENT_SCOPE)
    elseif (arch STREQUAL "x86_64")
        set(${out} "x86_64-apple-darwin20" PARENT_SCOPE)
    elseif (arch STREQUAL "arm64")
        set(${out} "aarch64-apple-darwin20" PARENT_SCOPE)
    else ()
        message(FATAL_ERROR "darling_arch_triple: unknown architecture '${arch}'")
    endif ()
endfunction()

# darling_arch_is_64bit(<arch> <out-var>): TRUE for the 64-bit slices.
function(darling_arch_is_64bit arch out)
    if (arch STREQUAL "i386")
        set(${out} FALSE PARENT_SCOPE)
    elseif (arch STREQUAL "x86_64" OR arch STREQUAL "arm64")
        set(${out} TRUE PARENT_SCOPE)
    else ()
        message(FATAL_ERROR "darling_arch_is_64bit: unknown architecture '${arch}'")
    endif ()
endfunction()

# darling_arch_family(<arch> <out-var>): "x86" for i386 and x86_64, "arm" for arm64. Sources and
# options for one instruction set (SSE options, x86 assembly) go to the slices of that family.
function(darling_arch_family arch out)
    if (arch STREQUAL "i386" OR arch STREQUAL "x86_64")
        set(${out} "x86" PARENT_SCOPE)
    elseif (arch STREQUAL "arm64")
        set(${out} "arm" PARENT_SCOPE)
    else ()
        message(FATAL_ERROR "darling_arch_family: unknown architecture '${arch}'")
    endif ()
endfunction()

# darling_arch_objc_abi(<arch> <out-var>): "legacy" (the 32-bit Objective-C runtime) or "modern".
function(darling_arch_objc_abi arch out)
    darling_arch_is_64bit(${arch} is_64bit)
    if (is_64bit)
        set(${out} "modern" PARENT_SCOPE)
    else ()
        set(${out} "legacy" PARENT_SCOPE)
    endif ()
endfunction()

# darling_arch_from_processor(<processor> <out-var>): the slice that matches a CMAKE_SYSTEM_PROCESSOR
# value (the architecture of the Linux side of Darling), or "" when no slice does.
function(darling_arch_from_processor processor out)
    if (processor MATCHES "^(aarch64|arm64)$")
        set(${out} "arm64" PARENT_SCOPE)
    elseif (processor MATCHES "^(x86[-_]64|amd64|AMD64)$")
        set(${out} "x86_64" PARENT_SCOPE)
    elseif (processor MATCHES "^i[3-6]86$")
        set(${out} "i386" PARENT_SCOPE)
    else ()
        set(${out} "" PARENT_SCOPE)
    endif ()
endfunction()

# darling_archs(<out-var> [32BIT_ONLY] [64BIT_ONLY] [FAMILY <x86|arm>] [ARCHS <arch>...])
#
# The enabled slices (DARLING_ARCHS, or ARCHS when given), in the canonical order, keeping only the
# 32-bit or 64-bit ones and only those of one FAMILY when asked to. The result may be empty.
function(darling_archs out)
    cmake_parse_arguments(SEL "32BIT_ONLY;64BIT_ONLY" "FAMILY" "ARCHS" ${ARGN})
    if (SEL_32BIT_ONLY AND SEL_64BIT_ONLY)
        message(FATAL_ERROR "darling_archs: 32BIT_ONLY and 64BIT_ONLY exclude each other")
    endif ()
    if (DEFINED SEL_ARCHS OR "ARCHS" IN_LIST ARGN)
        set(candidates ${SEL_ARCHS})
    else ()
        set(candidates ${DARLING_ARCHS})
    endif ()
    set(result "")
    foreach (arch IN LISTS candidates)
        darling_arch_is_64bit(${arch} is_64bit)
        darling_arch_family(${arch} family)
        if ((SEL_32BIT_ONLY AND is_64bit) OR (SEL_64BIT_ONLY AND NOT is_64bit))
            continue()
        endif ()
        if (SEL_FAMILY AND NOT family STREQUAL SEL_FAMILY)
            continue()
        endif ()
        list(APPEND result ${arch})
    endforeach ()
    set(${out} "${result}" PARENT_SCOPE)
endfunction()

# darling_arch_flags(<out-var> <arch>...): "-arch <a> -arch <b> ...", as one string for the
# COMPILE_FLAGS and LINK_FLAGS properties.
function(darling_arch_flags out)
    set(flags "")
    foreach (arch IN LISTS ARGN)
        string(APPEND flags " -arch ${arch}")
    endforeach ()
    string(STRIP "${flags}" flags)
    set(${out} "${flags}" PARENT_SCOPE)
endfunction()

# darling_xarch_options(<out-var> ARCHS <arch>... OPTIONS <option>...)
#
# The options for target_compile_options(), add_compile_options() or target_link_options(), each
# as "SHELL:-Xarch_<arch> <option>" for each of ARCHS, so that a fat compile passes them to those
# slices only. (The SHELL: form keeps each pair together; CMake removes repeated options, which would
# separate a bare -Xarch_<arch> from its option.) Options that take a separate argument must be
# written in their joined form (-DNAME=value, -Wl,a,b).
function(darling_xarch_options out)
    cmake_parse_arguments(XARCH "" "" "ARCHS;OPTIONS" ${ARGN})
    set(result "")
    foreach (arch IN LISTS XARCH_ARCHS)
        foreach (option IN LISTS XARCH_OPTIONS)
            list(APPEND result "SHELL:-Xarch_${arch} ${option}")
        endforeach ()
    endforeach ()
    set(${out} "${result}" PARENT_SCOPE)
endfunction()

# darling_xarch_flags(<out-var> ARCHS <arch>... OPTIONS <option>...): the same as one string,
# "-Xarch_<arch> <option> ...", for the COMPILE_FLAGS and LINK_FLAGS properties.
function(darling_xarch_flags out)
    cmake_parse_arguments(XARCH "" "" "ARCHS;OPTIONS" ${ARGN})
    set(result "")
    foreach (arch IN LISTS XARCH_ARCHS)
        foreach (option IN LISTS XARCH_OPTIONS)
            string(APPEND result " -Xarch_${arch} ${option}")
        endforeach ()
    endforeach ()
    string(STRIP "${result}" result)
    set(${out} "${result}" PARENT_SCOPE)
endfunction()

# darling_lipo(<name> OUTPUT <file> SLICES <target>... [IMPORTED <SHARED|STATIC>] [EXCLUDE_FROM_ALL])
#
# Joins the files of the SLICES targets, one or more thin (or fat) Mach-O files of distinct
# architectures, into the fat file OUTPUT with lipo. It is redone whenever one of the slices changes.
#
# Without IMPORTED, <name> is the custom target that produces OUTPUT. With IMPORTED, <name> is an
# imported global SHARED or STATIC library located at OUTPUT, to link other targets against, and
# <name>_lipo is the custom target producing it.
function(darling_lipo name)
    cmake_parse_arguments(LIPO "EXCLUDE_FROM_ALL" "OUTPUT;IMPORTED" "SLICES" ${ARGN})
    if (NOT LIPO_OUTPUT OR NOT LIPO_SLICES)
        message(FATAL_ERROR "darling_lipo(${name}): OUTPUT and SLICES are required")
    endif ()
    if (LIPO_IMPORTED AND NOT LIPO_IMPORTED MATCHES "^(SHARED|STATIC)$")
        message(FATAL_ERROR "darling_lipo(${name}): IMPORTED must be SHARED or STATIC")
    endif ()

    set(inputs "")
    foreach (slice IN LISTS LIPO_SLICES)
        list(APPEND inputs "$<TARGET_FILE:${slice}>")
    endforeach ()
    set(lipo "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/lipo")

    # The command's output is a stamp, with the fat file as a byproduct: CMake takes a dependency on
    # the file an imported target is located at for a dependency on that target, and would then drop
    # a rule that has the file as its OUTPUT.
    get_filename_component(output_name "${LIPO_OUTPUT}" NAME)
    set(stamp "${LIPO_OUTPUT}.lipo-stamp")
    add_custom_command(OUTPUT "${stamp}"
        BYPRODUCTS "${LIPO_OUTPUT}"
        COMMAND "${lipo}" -create ${inputs} -output "${LIPO_OUTPUT}"
        COMMAND "${CMAKE_COMMAND}" -E touch "${stamp}"
        DEPENDS ${LIPO_SLICES} lipo
        COMMENT "Running lipo to create ${output_name}"
        VERBATIM)

    if (LIPO_EXCLUDE_FROM_ALL)
        set(all_arg "")
    else ()
        set(all_arg ALL)
    endif ()
    if (LIPO_IMPORTED)
        add_custom_target(${name}_lipo ${all_arg} DEPENDS "${stamp}")
        add_library(${name} ${LIPO_IMPORTED} IMPORTED GLOBAL)
        set_target_properties(${name} PROPERTIES IMPORTED_LOCATION "${LIPO_OUTPUT}")
        add_dependencies(${name} ${name}_lipo)
    else ()
        add_custom_target(${name} ${all_arg} DEPENDS "${stamp}")
    endif ()
endfunction()

# _darling_normalize_archs(<out-var> <arch>...): the list in canonical order without duplicates;
# an unknown architecture or an empty list is a configure error.
function(_darling_normalize_archs out)
    set(requested ${ARGN})
    if (NOT requested)
        message(FATAL_ERROR "DARLING_ARCHS is empty; enable at least one of: ${DARLING_SUPPORTED_ARCHS}")
    endif ()
    foreach (arch IN LISTS requested)
        if (NOT arch IN_LIST DARLING_SUPPORTED_ARCHS)
            message(FATAL_ERROR "DARLING_ARCHS: unknown architecture '${arch}' (supported: ${DARLING_SUPPORTED_ARCHS})")
        endif ()
    endforeach ()
    set(result "")
    foreach (arch IN LISTS DARLING_SUPPORTED_ARCHS)
        if (arch IN_LIST requested)
            list(APPEND result ${arch})
        endif ()
    endforeach ()
    set(${out} "${result}" PARENT_SCOPE)
endfunction()

# _darling_primary_arch(<out-var> <processor> <arch>...): the slice matching the Linux side of
# Darling when it is enabled, otherwise the first 64-bit slice, otherwise the first slice.
function(_darling_primary_arch out processor)
    set(archs ${ARGN})
    darling_arch_from_processor("${processor}" native)
    if (native AND native IN_LIST archs)
        set(${out} ${native} PARENT_SCOPE)
        return()
    endif ()
    darling_archs(archs_64 64BIT_ONLY ARCHS ${archs})
    if (archs_64)
        list(GET archs_64 0 primary)
    else ()
        list(GET archs 0 primary)
    endif ()
    set(${out} ${primary} PARENT_SCOPE)
endfunction()

# generate_architecture(): checks DARLING_ARCHS and puts it in the canonical order, then sets the
# variables derived from it:
#
#   DARLING_ARCHS_32BIT, DARLING_ARCHS_64BIT   the enabled 32-bit and 64-bit slices
#   APPLE_ARCH_PRIMARY                          the slice of the build tools' defaults (lipo, ar, the
#                                               default -target), see _darling_primary_arch
#   APPLE_TARGET_TRIPLET_PRIMARY                its triple
#   TARGET_i386, TARGET_x86_64, TARGET_ARM64   TRUE when that slice is enabled
#   BUILD_TARGET_32BIT, BUILD_TARGET_64BIT     TRUE when a 32-bit / 64-bit slice is enabled
#
# and, for components that still assume at most one slice of each width:
#
#   APPLE_ARCH_32BIT, APPLE_TARGET_TRIPLET_32BIT   i386 when enabled, otherwise ""
#   APPLE_ARCH_64BIT, APPLE_TARGET_TRIPLET_64BIT   the primary 64-bit slice (the primary slice when it
#                                                  is 64-bit, otherwise the first one), or ""
#
# These are only correct with at most one 64-bit slice; new code loops over DARLING_ARCHS instead.
macro(generate_architecture)
    _darling_normalize_archs(DARLING_ARCHS ${DARLING_ARCHS})
    darling_archs(DARLING_ARCHS_32BIT 32BIT_ONLY)
    darling_archs(DARLING_ARCHS_64BIT 64BIT_ONLY)
    _darling_primary_arch(APPLE_ARCH_PRIMARY "${CMAKE_SYSTEM_PROCESSOR}" ${DARLING_ARCHS})
    darling_arch_triple(${APPLE_ARCH_PRIMARY} APPLE_TARGET_TRIPLET_PRIMARY)

    foreach (_darling_arch IN ITEMS i386 x86_64 ARM64)
        string(TOLOWER "${_darling_arch}" _darling_arch_lc)
        if (_darling_arch_lc IN_LIST DARLING_ARCHS)
            set(TARGET_${_darling_arch} TRUE)
        else ()
            set(TARGET_${_darling_arch} FALSE)
        endif ()
    endforeach ()
    unset(_darling_arch)
    unset(_darling_arch_lc)

    if (DARLING_ARCHS_32BIT)
        set(BUILD_TARGET_32BIT TRUE)
        list(GET DARLING_ARCHS_32BIT 0 APPLE_ARCH_32BIT)
        darling_arch_triple(${APPLE_ARCH_32BIT} APPLE_TARGET_TRIPLET_32BIT)
    else ()
        set(BUILD_TARGET_32BIT FALSE)
        set(APPLE_ARCH_32BIT "")
        set(APPLE_TARGET_TRIPLET_32BIT "")
    endif ()

    if (DARLING_ARCHS_64BIT)
        set(BUILD_TARGET_64BIT TRUE)
        if (APPLE_ARCH_PRIMARY IN_LIST DARLING_ARCHS_64BIT)
            set(APPLE_ARCH_64BIT ${APPLE_ARCH_PRIMARY})
        else ()
            list(GET DARLING_ARCHS_64BIT 0 APPLE_ARCH_64BIT)
        endif ()
        darling_arch_triple(${APPLE_ARCH_64BIT} APPLE_TARGET_TRIPLET_64BIT)
    else ()
        set(BUILD_TARGET_64BIT FALSE)
        set(APPLE_ARCH_64BIT "")
        set(APPLE_TARGET_TRIPLET_64BIT "")
    endif ()
endmacro()
