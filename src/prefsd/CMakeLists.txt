project(darling-prefsd)

add_definitions(-nostdinc)

# dserver_platform_identity_t and dserver_system_config_t, used by
# libsystem_kernel's __darling_set_platform_identity and __darling_set_system_config
include_directories(${CMAKE_SOURCE_DIR}/src/external/darlingserver/include)

get_property(DARLING_OS_PRODUCT_VERSION GLOBAL PROPERTY DARLING_OS_PRODUCT_VERSION)
get_property(DARLING_OS_BUILD_VERSION GLOBAL PROPERTY DARLING_OS_BUILD_VERSION)
get_property(DARLING_KERNEL_RELEASE GLOBAL PROPERTY DARLING_KERNEL_RELEASE)

add_darling_executable(darling-prefsd darling-prefsd.c prefsd-validate.c)
target_compile_definitions(darling-prefsd PRIVATE
	DARLING_DEFAULT_OS_PRODUCT_VERSION="${DARLING_OS_PRODUCT_VERSION}"
	DARLING_DEFAULT_OS_BUILD_VERSION="${DARLING_OS_BUILD_VERSION}"
	DARLING_DEFAULT_KERNEL_RELEASE="${DARLING_KERNEL_RELEASE}"
)
target_link_libraries(darling-prefsd CoreFoundation)

install(TARGETS darling-prefsd DESTINATION libexec/darling/usr/libexec)
install(FILES org.darlinghq.prefsd.plist DESTINATION libexec/darling/System/Library/LaunchDaemons)
