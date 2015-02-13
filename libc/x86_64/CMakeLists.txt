project(libc-x86_64)

cmake_minimum_required(VERSION 3.1.0)
enable_language(C ASM)

add_definitions(-D_getcontext=getcontext)
add_definitions(-D_moncount=moncount)
add_definitions(-D__setcontext=_setcontext)
add_definitions(-D_getmcontext=getmcontext)
add_definitions(-D__ctx_done=_ctx_done)
add_definitions(-D__ctx_start=_ctx_start)
add_definitions(-D_cpu_number=cpu_number)
add_definitions(-D_sys_icache_invalidate=sys_icache_invalidate)
add_definitions(-D_sys_dcache_flush=sys_dcache_flush)

#set(CMAKE_C_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/../../duct/include/commpage.h -include ${CMAKE_CURRENT_SOURCE_DIR}/../../platform-include/sys/time.h")

set(x86_64_sources
	gen/getmcontext.c
	gen/makecontext.c
	gen/setcontext.c
	gen/setjmperr.c
	gen/swapcontext.c
	gen/cpu_number.S
	gen/icacheinval.S
	gen/mcount.S
	gen/_ctx_start.S
	gen/getcontext.S
	gen/_setcontext.S
	../i386/pthreads/init_cpu_capabilities.c
)

add_definitions(-D_preempt=preempt)
add_definitions(-D_backoff=backoff)
add_definitions(-D_pthread_getspecific=pthread_getspecific)
add_definitions(-D__commpage_pthread_mutex_lock=_commpage_pthread_mutex_lock)
add_definitions(-D_pthread_self=pthread_self)
add_definitions(-D___pthread_set_self=__pthread_set_self)
add_definitions(-D_start_wqthread=start_wqthread)
add_definitions(-D_thread_start=thread_start)
add_definitions(-D__pthread_start=_pthread_start)
add_definitions(-D__pthread_wqthread=_pthread_wqthread)

set(x86_64_sources ${x86_64_sources}
	pthreads/preempt.S
	pthreads/pthread_getspecific.S
	pthreads/pthread_mutex_lock.S
	pthreads/pthread_self.S
	pthreads/pthread_set_self.S
	pthreads/start_wqthread.S
	pthreads/thread_start.S
)

set(x86_64_sources ${x86_64_sources}
	string/bcopy.c
	string/bcopy_sse3x.S
	string/bcopy_sse42.S
	string/bzero.c
	string/__bzero.S
	string/bzero_sse2.S
	string/bzero_sse42.S
	string/ffs.S
	string/longcopy_sse3x.S
	string/memcmp.S
	string/memcpy.c
	string/memmove.c
	string/memset.S
	string/strcmp.S
	string/strcpy.S
	string/strlcat.S
	string/strlcpy.S
	string/strlen.S
	string/strncmp.S
	string/strncpy.S
)

add_library(libc-hwplat OBJECT ${x86_64_sources})


