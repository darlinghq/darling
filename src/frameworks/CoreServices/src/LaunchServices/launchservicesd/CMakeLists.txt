project(launchservicesd)

include_directories(
	${CMAKE_SOURCE_DIR}/src/external/fmdb/src
)

set(sources
	launchservicesd.m
	LSBundle.m
)

add_darling_executable(launchservicesd ${sources})

target_link_libraries(launchservicesd system Foundation CoreFoundation CoreServices FMDB sqlite3 z)
install(TARGETS launchservicesd DESTINATION libexec/darling/System/Library/CoreServices)
install(FILES schema.sql RENAME launchservicesd-schema.sql DESTINATION libexec/darling/System/Library/CoreServices)
