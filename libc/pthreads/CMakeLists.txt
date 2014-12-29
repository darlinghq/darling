project(libc-pthreads)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/../../duct/include/commpage.h -include ${CMAKE_CURRENT_SOURCE_DIR}/../../platform-include/sys/time.h")

set(pthreads_sources
	pthread.c
	pthread_cancelable.c
	pthread_cond.c
	pthread_mutex.c
	pthread_rwlock.c
	pthread_tsd.c
	thread_setup.c
)

SET_SOURCE_FILES_PROPERTIES(pthread.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_PTHREAD_CANCEL -DLIBC_ALIAS_PTHREAD_SETCANCELSTATE -DLIBC_ALIAS_PTHREAD_SETCANCELTYPE -DLIBC_ALIAS_PTHREAD_SIGMASK -DLIBC_ALIAS_PTHREAD_TESTCANCEL")
SET_SOURCE_FILES_PROPERTIES(pthread_cancelable.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_PTHREAD_COND_TIMEDWAIT -DLIBC_ALIAS_PTHREAD_COND_WAIT -DLIBC_ALIAS_PTHREAD_JOIN -DLIBC_ALIAS_SIGWAIT")
SET_SOURCE_FILES_PROPERTIES(thread_setup.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0")

add_library(libc-pthreads OBJECT ${pthreads_sources})

set(pthreads-cancelable_sources
	pthread_cancelable.c)

add_library(libc-pthreads_cancelable OBJECT ${pthreads-cancelable_sources})
SET_TARGET_PROPERTIES(libc-pthreads_cancelable PROPERTIES COMPILE_DEFINITIONS "BUILDING_VARIANT;VARIANT_CANCELABLE")

