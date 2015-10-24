project(libc-i386)

cmake_minimum_required(VERSION 3.1.0)
enable_language(C ASM)

#set(CMAKE_C_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/../../duct/include/commpage.h -include ${DARLING_TOP_DIRECTORY}/platform-include/sys/time.h")

set(i386_sources
	gen/getmcontext.c
	gen/makecontext.c
	gen/setcontext.c
	gen/setjmperr.c
	gen/swapcontext.c
	gen/cpu_number.s
	gen/icacheinval.s
	gen/mcount.s
	gen/_ctx_start.S
	gen/getcontext.S
	gen/_setcontext.S
)

add_library(libc-hwplat OBJECT ${i386_sources})


