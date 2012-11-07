project(crash)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

#if (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")
#	message(FATAL_ERROR "Clang is the only supported compiler.")
#endif (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")

# CMake is broken and uses C++ flags for ObjC
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")

set(crash_SRCS
	AppController.m
	CrashManager.m
	crash_main.m
)

add_executable(crash ${crash_SRCS})
target_link_libraries(crash -lgnustep-base -lgnustep-gui)

install(TARGETS crash DESTINATION "libexec/darling/crash.app/")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Main.gsmarkup"
	"${CMAKE_CURRENT_SOURCE_DIR}/Resources/Info-gnustep.plist"
	DESTINATION "libexec/darling/crash.app/Resources/")

FILE( GLOB gorm ${CMAKE_CURRENT_SOURCE_DIR}/Resources/crash.gorm/* )
install(FILES ${gorm} DESTINATION
	"libexec/darling/crash.app/Resources/crash.gorm/")

