project(libc-stdlib)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -D__DYNAMIC__ -D__BLOCKS__ -fblocks")

set(stdlib_sources a64l.c grantpt.c l64a.c qsort_b-fbsd.c)

set(stdlib_sources ${stdlib_sources}
	FreeBSD/abort.c
	FreeBSD/abs.c
	FreeBSD/atexit.c
	FreeBSD/atof.c
	FreeBSD/atoi.c
	FreeBSD/atol.c
	FreeBSD/atoll.c
	FreeBSD/bsearch.c
	FreeBSD/div.c
	FreeBSD/exit.c
	FreeBSD/_Exit_.c
	FreeBSD/getenv.c
	FreeBSD/getopt.c
	FreeBSD/getopt_long.c
	FreeBSD/getsubopt.c
	FreeBSD/hcreate.c
	FreeBSD/heapsort_b.c
	FreeBSD/heapsort.c
	FreeBSD/heapsort_r.c
	FreeBSD/imaxabs.c
	FreeBSD/imaxdiv.c
	FreeBSD/insque.c
	FreeBSD/labs.c
	FreeBSD/ldiv.c
	FreeBSD/llabs.c
	FreeBSD/lldiv.c
	FreeBSD/lsearch.c
	FreeBSD/merge_b.c
	FreeBSD/merge.c
	FreeBSD/psort_b.c
	FreeBSD/psort.c
	FreeBSD/psort_r.c
	FreeBSD/putenv.c
	#FreeBSD/qsort.c
	#FreeBSD/qsort_r.c
	FreeBSD/radixsort.c
	FreeBSD/rand.c
	FreeBSD/random.c
	FreeBSD/reallocf.c
	FreeBSD/realpath.c                                                                                                                  
	FreeBSD/remque.c                                                                                                                    
	FreeBSD/setenv.c                                                                                                                    
	FreeBSD/strhash.c
	FreeBSD/strtoimax.c
	FreeBSD/strtol.c
	FreeBSD/strtoll.c
	FreeBSD/strtoq.c
	FreeBSD/strtoul.c
	FreeBSD/strtoull.c
	FreeBSD/strtoumax.c
	FreeBSD/strtouq.c
	FreeBSD/system.c
	FreeBSD/tdelete.c
	FreeBSD/tfind.c
	FreeBSD/tsearch.c
	FreeBSD/twalk.c
)

set(stdlib_sources ${stdlib_sources}
	OpenBSD/ecvt.c
	OpenBSD/gcvt.c
)

set(stdlib_sources ${stdlib_sources}
	NetBSD/strfmon.c
)

#SET_SOURCE_FILES_PROPERTIES(FreeBSD/killpg.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_KILLPG")

add_library(libc-stdlib OBJECT ${stdlib_sources})
