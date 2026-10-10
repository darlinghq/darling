set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1239.200.12")

set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(FSEvents
	BASE_PATH ${coreservices_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "../../include/FSEvents"
)

add_framework(FSEvents
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "CoreServices"
	SOURCES
		FSEvents.m
		FSEventsImpl.m
	DEPENDENCIES
		CoreFoundation
		Foundation
		system
)

add_darling_executable(fseventsd fseventsd.m)
target_link_libraries(fseventsd system Foundation CarbonCore)

install(TARGETS fseventsd DESTINATION libexec/darling/usr/sbin)
include(setcap)
setcap(libexec/darling/usr/sbin/fseventsd "cap_sys_admin+ep")
