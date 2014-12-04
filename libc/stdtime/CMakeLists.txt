project(libc-stdtime)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -include stdbool.h")

set(stdtime_sources getdate.c timezone_unix03.c)

set(stdtime_sources ${stdtime_sources}
	FreeBSD/asctime.c
	FreeBSD/difftime.c
	FreeBSD/ftime.c
	FreeBSD/localtime.c
	FreeBSD/strftime.c
	FreeBSD/strptime.c
	FreeBSD/time32.c
	FreeBSD/timelocal.c
)

add_library(libc-stdtime OBJECT ${stdtime_sources})
