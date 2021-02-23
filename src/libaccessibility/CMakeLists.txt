project(Accessibility)

set(DYLIB_INSTALL_NAME "/usr/lib/libAccessibility.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(Accessibility SHARED
	src/Accessibility.m
	src/AccessibilitySupportOverrides.m
	src/AXDefaultsObserverPostDarwinNotificationAction.m
	src/AXDefaultsObserverExecuteBlockNotificationAction.m
	src/AXSupportDefaultsObserver.m
)
make_fat(Accessibility)
target_link_libraries(Accessibility system objc Foundation)
install(TARGETS Accessibility DESTINATION libexec/darling/usr/lib)
