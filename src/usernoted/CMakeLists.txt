project(usernoted)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/src/native/include
)

add_darling_executable(usernoted usernoted.m usernoted_dbus.c)
target_link_libraries(usernoted Foundation)

install(TARGETS usernoted DESTINATION libexec/darling/usr/libexec)
install(FILES com.apple.usernoted.plist DESTINATION libexec/darling/System/Library/LaunchDaemons)
