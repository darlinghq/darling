project(libc-i386)

cmake_minimum_required(VERSION 3.1.0)
enable_language(C ASM)

#set(CMAKE_C_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/../../duct/include/commpage.h -include ${DARLING_TOP_DIRECTORY}/platform-include/sys/time.h")

add_definitions(-DPRIVATE)

set(i386_sources
	gen/getmcontext.c
	#gen/makecontext.c
	#gen/setcontext.c
	gen/setjmperr.c
	#gen/swapcontext.c
	gen/cpu_number.S
	gen/icacheinval.S
	gen/mcount.S
	gen/_ctx_start.S
	#gen/getcontext.S
	gen/_setcontext.S

	# sys/i386_gettimeofday_asm.S
	# sys/mach_absolute_time_asm.S
	sys/atomic.c
	sys/spinlocks.c
	sys/OSAtomic.S
	sys/_setjmp.S
	sys/setjmp.S
	sys/_sigtramp.S
	sys/spinlocks_asm.S

	string/bcopy_scalar.S
	string/bcopy_sse2.S
	string/bcopy_sse3x.S
	string/bcopy_sse42.S
	string/__bzero.S
	string/bzero_scalar.S
	string/bzero_sse2.S
	string/bzero_sse42.S
	string/ffs.S
	string/longcopy_sse3x.S
	string/memcmp.S
	string/memset_pattern_sse2.S
	string/memset.S
	string/strcmp.S
	string/strcpy.S
	string/strlcat.S
	string/strlcpy.S
	string/strlen.S
	string/strncmp.S
	string/strncpy.S
	string/bcopy.c
	string/bzero.c
	string/memcpy.c
	string/memmove.c

	# pthreads/preempt.S
	pthreads/pthread_getspecific.S
	pthreads/pthread_mutex_lock.S
	pthreads/pthread_self.S
	pthreads/pthread_set_self.S
	pthreads/start_wqthread.S
	pthreads/thread_start.S
	pthreads/init_cpu_capabilities.c
)

add_library(libc-hwplat OBJECT ${i386_sources})


