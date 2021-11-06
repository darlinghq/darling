project(mig)

cmake_minimum_required(VERSION 3.10)

find_package(BISON)
find_package(FLEX)

if (TARGET_ARM64)
	# For some reason, Linux and MacOS uses different macros when refering to
	# the arm64 architecture. The macro is `__aarch64__` on Linux, while the
	# macro is __arm64__ on MacOS.
	
	add_definitions(-D__arm64__)
endif (TARGET_ARM64)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
add_definitions(-Du_int=unsigned -D__int64_t=int64_t -D__int32_t=int32_t -D__darwin_natural_t=unsigned -D__uint32_t=uint32_t -D__uint16_t=uint16_t -D__uint64_t=uint64_t -D_BSD_I386__TYPES_H_)
add_definitions('-DMIG_VERSION="1.0"')
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/mach)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/migcom.tproj)

BISON_TARGET(MyParser migcom.tproj/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c)
FLEX_TARGET(MyScanner migcom.tproj/lexxer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.c)
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

# Hide some unimportant warnings
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-dangling-else -Wno-implicit-function-declaration")

set(mig_sources
	migcom.tproj/error.c
	migcom.tproj/global.c
	#migcom.tproj/handler.c
	migcom.tproj/header.c
	migcom.tproj/mig.c
	migcom.tproj/routine.c
	migcom.tproj/server.c
	migcom.tproj/statement.c
	migcom.tproj/string.c
	migcom.tproj/type.c
	migcom.tproj/user.c
	migcom.tproj/utils.c
	migcom.tproj/xtracemig.c
	${BISON_MyParser_OUTPUTS}
    ${FLEX_MyScanner_OUTPUTS}
)

add_executable(migcom ${mig_sources})

add_custom_command(
	#TARGET migcom
	#POST_BUILD
	#DEPENDS migcom
	OUTPUT "${CMAKE_BINARY_DIR}/build-mig"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/migcom.tproj/mig.sh"
	COMMAND
	awk -v "migcc=${CMAKE_C_COMPILER}"
	-v "migcom=${CMAKE_CURRENT_BINARY_DIR}/migcom"
	-f "${CMAKE_CURRENT_SOURCE_DIR}/migcom.tproj/mig.awk"
	"${CMAKE_CURRENT_SOURCE_DIR}/migcom.tproj/mig.sh" ">${CMAKE_BINARY_DIR}/build-mig"
	&& chmod 0755 "${CMAKE_BINARY_DIR}/build-mig"
	)
add_custom_target(migexe DEPENDS "${CMAKE_BINARY_DIR}/build-mig")

add_dependencies(migexe migcom)


