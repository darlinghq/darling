project(libc-regex)

cmake_minimum_required(VERSION 2.4.0)

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

#SET_SOURCE_FILES_PROPERTIES(FreeBSD/snprintf.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS}")

add_library(libc-regex OBJECT ${regex_sources})

