project(libc-threads)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/../fbsdcompat/_fbsd_compat_.h")

set(threads_sources
	cprocs.c
	cthreads.c
	mig_support.c
)

add_library(libc-threads OBJECT ${threads_sources})
