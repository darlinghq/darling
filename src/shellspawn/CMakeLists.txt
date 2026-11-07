project(shellspawn)

add_definitions(-nostdinc)

# dserver_system_config_t, used by libsystem_kernel's __darling_wait_system_config
include_directories(${CMAKE_SOURCE_DIR}/src/external/darlingserver/include)

add_darling_executable(shellspawn shellspawn.c shellspawn-config.c duct_signals.c)

install(TARGETS shellspawn DESTINATION libexec/darling/usr/libexec)
install(FILES org.darlinghq.shellspawn.plist DESTINATION libexec/darling/System/Library/LaunchDaemons)

