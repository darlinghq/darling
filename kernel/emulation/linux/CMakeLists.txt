project(emulation)

cmake_minimum_required(VERSION 2.4.0)

enable_language(C ASM)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -fvisibility=hidden -fPIC -ggdb")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

set(emulation_sources
	base.c
	debug.c
	errno.c
	write.c
	syscalls-amd64.asm
)

add_library(emulation OBJECT ${emulation_sources})
