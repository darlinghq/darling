project(libc-regex)

cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -D_C99_SOURCE -D_FORTIFY_SOURCE=0 -D__va_list=__builtin_va_list -DHAVE_CONFIG_H")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/TRE")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/FreeBSD")

set(regex_sources
	FreeBSD/regerror.c
	TRE/lib/regcomp.c
	TRE/lib/regexec.c
	TRE/lib/tre-ast.c
	TRE/lib/tre-compile.c
	TRE/lib/tre-match-backtrack.c
	TRE/lib/tre-match-parallel.c
	TRE/lib/tre-mem.c
	TRE/lib/tre-parse.c
	TRE/lib/tre-stack.c
)

SET_SOURCE_FILES_PROPERTIES(TRE/lib/regcomp.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_REGCOMP")

add_darling_object_library(libc-regex ${regex_sources})

# Legacy variant
if (TARGET_i386)
	set(regex-legacy_sources
		TRE/lib/regcomp.c)
	add_library(libc-regex_legacy OBJECT ${regex-legacy_sources})
	SET_TARGET_PROPERTIES(libc-regex_legacy PROPERTIES COMPILE_FLAGS
		"-DBUILDING_VARIANT -DVARIANT_LEGACY -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0")
endif (TARGET_i386)
