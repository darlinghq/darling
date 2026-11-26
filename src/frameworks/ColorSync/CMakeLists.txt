project(ColorSync)

remove_sdk_framework(ColorSync)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "4.7.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ColorSync
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ColorSync"
)

add_framework(ColorSync
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/ColorSync.c
        src/ColorSyncProfile.c
        src/ICCBuilder.c
        src/ICCProfile.c

    DEPENDENCIES
        CoreFoundation
        system
)

# The profile files of the ColorSync profile folder, which applications open by path. They are written by a tool that runs on the
# build machine (compiled with the unflagged compiler, like the OpenAL tables) from the same code and numbers as
# ColorSyncProfileCreateWithName(), so file and name give the same profile.
set(colorsync_gen_dir "${CMAKE_CURRENT_BINARY_DIR}/gen")
add_custom_command(
    OUTPUT "${colorsync_gen_dir}/genericrgb.icc" "${colorsync_gen_dir}/genericgray.icc"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${colorsync_gen_dir}"
    COMMAND ${CMAKE_C_COMPILER} -O1 -I "${CMAKE_CURRENT_SOURCE_DIR}/src"
        -o "${colorsync_gen_dir}/genprofile"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/genprofile.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/ICCBuilder.c"
    COMMAND "${colorsync_gen_dir}/genprofile" genericrgb "${colorsync_gen_dir}/genericrgb.icc"
    COMMAND "${colorsync_gen_dir}/genprofile" genericgray "${colorsync_gen_dir}/genericgray.icc"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/genprofile.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ICCBuilder.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ICCBuilder.h"
    VERBATIM
)
add_custom_target(colorsync_profile_files ALL
    DEPENDS "${colorsync_gen_dir}/genericrgb.icc" "${colorsync_gen_dir}/genericgray.icc")
# Building the framework target alone (ninja ColorSync) must also produce the files, or the install rules below fail on files that
# were never generated and the profile folder is left without them.
add_dependencies(ColorSync colorsync_profile_files)

install(FILES "${colorsync_gen_dir}/genericrgb.icc"
    DESTINATION libexec/darling/System/Library/ColorSync/Profiles
    RENAME "Generic RGB Profile.icc")
install(FILES "${colorsync_gen_dir}/genericgray.icc"
    DESTINATION libexec/darling/System/Library/ColorSync/Profiles
    RENAME "Generic Gray Profile.icc")
