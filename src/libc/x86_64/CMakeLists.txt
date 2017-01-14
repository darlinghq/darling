project(libc-x86_64)

cmake_minimum_required(VERSION 2.8.0)
enable_language(C ASM)

#add_definitions(-D_getcontext=getcontext)
#add_definitions(-D_moncount=moncount)
#add_definitions(-D__setcontext=_setcontext)
#add_definitions(-D_getmcontext=getmcontext)
#add_definitions(-D__ctx_done=_ctx_done)
#add_definitions(-D__ctx_start=_ctx_start)
#add_definitions(-D_cpu_number=cpu_number)
#add_definitions(-D_sys_icache_invalidate=sys_icache_invalidate)
#add_definitions(-D_sys_dcache_flush=sys_dcache_flush)
add_definitions(-DPRIVATE)

#set(CMAKE_C_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/../../duct/include/commpage.h -include ${DARLING_TOP_DIRECTORY}/platform-include/sys/time.h")

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

#add_definitions(-D_preempt=preempt)
#add_definitions(-D_backoff=backoff)
#add_definitions(-D_pthread_getspecific=pthread_getspecific)
#add_definitions(-D__commpage_pthread_mutex_lock=_commpage_pthread_mutex_lock)
#add_definitions(-D_pthread_self=pthread_self)
#add_definitions(-D___pthread_set_self=__pthread_set_self)
#add_definitions(-D_start_wqthread=start_wqthread)
#add_definitions(-D_thread_start=thread_start)
#add_definitions(-D__pthread_start=_pthread_start)
#add_definitions(-D__pthread_wqthread=_pthread_wqthread)

set(x86_64_sources ${x86_64_sources}
	pthreads/preempt.S
	pthreads/pthread_getspecific.S
	pthreads/pthread_mutex_lock.S
	pthreads/pthread_self.S
	#pthreads/pthread_set_self.S
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

add_definitions(-DkUP=0x8000)
set(x86_64_sources ${x86_64_sources}
	sys/atomic.c
	sys/i386_gettimeofday_asm.S
	#sys/nanotime.S # Fully dependent on commpage! TODO: reimplement in system_kernel
	sys/OSAtomic.S
	sys/_setjmp.S
	sys/setjmp.S
	sys/_sigtramp.S
	sys/spinlocks_asm.S
	sys/spinlocks.c # aliases in other complation units don't work, work done in spinlocks_asm.S
)


add_library(libc-hwplat OBJECT ${x86_64_sources})

add_library(libc-hwplat_dyld OBJECT
	#pthreads/pthread_set_self.S # in system_kernel
	pthreads/pthread_self.S
	pthreads/pthread_getspecific.S
	pthreads/preempt.S
	pthreads/thread_start.S
	pthreads/start_wqthread.S
	string/__bzero.S
	string/bcopy_sse3x.S
	string/bcopy_sse42.S
	string/bcopy.c
	string/bzero_sse2.S
	string/bzero_sse42.S
	string/bzero.c
	string/ffs.S
	string/longcopy_sse3x.S
	string/strcmp.S
	string/strlen.S
	string/strncpy.S
	string/memmove.c
	string/memcpy.c
	sys/OSAtomic.S
	sys/atomic.c
	sys/spinlocks.c
	sys/i386_gettimeofday_asm.S
	sys/spinlocks_asm.S)

set_target_properties(libc-hwplat_dyld PROPERTIES COMPILE_FLAGS "-DVARIANT_DYLD")

