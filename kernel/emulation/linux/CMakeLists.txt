project(emulation)

cmake_minimum_required(VERSION 2.4.0)

enable_language(C ASM)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -fvisibility=hidden -fPIC -ggdb")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

set(emulation_sources
	base.c
	syscalls.c
	debug.c
	errno.c
	unistd/write.c
	unistd/read.c
	unistd/pread.c
	unistd/pwrite.c
	mman.c
	unistd/fsync.c
	unistd/sync.c
	unistd/fdatasync.c
	unistd/dup.c
	unistd/dup2.c
	unistd/exit.c
	unistd/close.c
	unistd/fchdir.c
	unistd/fchown.c
	unistd/fchmod.c
	unistd/setgid.c
	unistd/setuid.c
	unistd/setegid.c
	unistd/seteuid.c
	unistd/setsid.c
	unistd/getuid.c
	unistd/geteuid.c
	unistd/getpid.c
	unistd/lseek.c
	unistd/ftruncate.c
	signal/duct_signals.c
	signal/kill.c
	syscalls-amd64.asm
)

set_source_files_properties(mman.c PROPERTIES COMPILE_FLAGS
	"-I${CMAKE_CURRENT_SOURCE_DIR}/../../../platform-include")
set_source_files_properties(signal/duct_signals.c PROPERTIES COMPILE_FLAGS
	"-I${CMAKE_CURRENT_SOURCE_DIR}/../../../platform-include")

add_library(emulation OBJECT ${emulation_sources})
