project(darling-prefsd)

add_definitions(-nostdinc)

# dserver_platform_identity_t and dserver_system_config_t, used by
# libsystem_kernel's __darling_set_platform_identity and __darling_set_system_config
include_directories(${CMAKE_SOURCE_DIR}/src/external/darlingserver/include)

add_darling_executable(darling-prefsd darling-prefsd.c prefsd-validate.c)
target_link_libraries(darling-prefsd CoreFoundation)

install(TARGETS darling-prefsd DESTINATION libexec/darling/usr/libexec)
install(FILES org.darlinghq.prefsd.plist DESTINATION libexec/darling/System/Library/LaunchDaemons)
