project(lslaunchservicesdd)

set(sources
	launchservicesd.cpp
)

add_darling_executable(launchservicesd ${sources})

target_link_libraries(launchservicesd system CoreServices sqlite3 cxx)
install(TARGETS launchservicesd DESTINATION libexec/darling/System/Library/CoreServices)
