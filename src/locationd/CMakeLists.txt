project(locationd)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/src/native/include
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_DBUS REQUIRED dbus-1)
include_directories(${PC_DBUS_INCLUDE_DIRS})

add_darling_executable(locationd locationd.m locationd_dbus.c)
target_link_libraries(locationd Foundation darling_dbus_loader)

install(TARGETS locationd DESTINATION libexec/darling/usr/libexec)
install(FILES com.apple.locationd.plist DESTINATION libexec/darling/System/Library/LaunchDaemons)
