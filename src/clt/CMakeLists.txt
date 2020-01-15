project(clt)

set(CLT_ROOT libexec/darling/Library/Developer/DarlingCLT)

install(DIRECTORY DESTINATION ${CLT_ROOT})

add_definitions(-DDEBUG)

add_darling_executable(xcrun_clt xcrun.c)
set_target_properties(xcrun_clt PROPERTIES OUTPUT_NAME xcrun)
install(TARGETS xcrun_clt DESTINATION ${CLT_ROOT}/usr/bin)
