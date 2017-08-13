project(sysmon)

set(DYLIB_INSTALL_NAME "/usr/lib/libsysmon.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

file(GLOB sysmon_objc "src/*.m")

add_darling_library(sysmon
	src/sysmon.c
	${sysmon_objc}
)
make_fat(sysmon)
target_link_libraries(sysmon system objc Foundation)

install(TARGETS sysmon DESTINATION libexec/darling/usr/lib)
