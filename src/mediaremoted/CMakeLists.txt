project(mediaremoted)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/src/native/include
)

# The host's libdbus-1, used through the dbus wrapper library
find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_DBUS REQUIRED dbus-1)
include_directories(${PC_DBUS_INCLUDE_DIRS})

add_darling_executable(mediaremoted mediaremoted.m mediaremoted_dbus.c)
target_link_libraries(mediaremoted Foundation dbus darling_dbus_loader)

install(TARGETS mediaremoted DESTINATION libexec/darling/usr/libexec)
install(FILES com.apple.mediaremoted.plist DESTINATION libexec/darling/System/Library/LaunchDaemons)
