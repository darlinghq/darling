project(libc-util)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(util_sources
	fparseln.c
	login_tty.c
	login.c
	logout.c
	logwtmp.c
	mkpath_np.c
	opendev.c
	pty.c)

add_library(libc-util OBJECT ${util_sources})
