project(libaccessibility)

set(DYLIB_INSTALL_NAME "/usr/lib/libAccessibility.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

# The target is not named "Accessibility" because that name belongs to the
# Accessibility.framework target; the output file is still libAccessibility.dylib.
add_darling_library(libaccessibility SHARED
	src/Accessibility.m
	src/AccessibilitySupportOverrides.m
	src/AXDefaultsObserverPostDarwinNotificationAction.m
	src/AXDefaultsObserverExecuteBlockNotificationAction.m
	src/AXSupportDefaultsObserver.m
)
set_target_properties(libaccessibility PROPERTIES OUTPUT_NAME Accessibility)
make_fat(libaccessibility)
target_link_libraries(libaccessibility system objc Foundation)
install(TARGETS libaccessibility DESTINATION libexec/darling/usr/lib)
