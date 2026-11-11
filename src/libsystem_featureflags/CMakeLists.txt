project(system_featureflags)

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -fPIC")

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_featureflags.dylib")
set(DYLIB_CURRENT_VERSION "108.0.0")
add_darling_library(system_featureflags SHARED feature.c)
make_fat(system_featureflags)

install(TARGETS system_featureflags DESTINATION libexec/darling/usr/lib/system)
