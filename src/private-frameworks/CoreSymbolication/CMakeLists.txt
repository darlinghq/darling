project(CoreSymbolication)

remove_sdk_framework(CoreSymbolication
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "62046.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CoreSymbolication
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CoreSymbolication"
	PRIVATE
)

add_framework(CoreSymbolication
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/functions.c

	DEPENDENCIES
		system
)

add_darling_executable(coresymbolicationd src/daemon.c)

install(
	TARGETS
		coresymbolicationd
	DESTINATION
		libexec/darling/usr/libexec
)

install(
	FILES
		resources/com.apple.coresymbolicationd.plist
	DESTINATION
		libexec/darling/System/Library/LaunchDaemons
)
