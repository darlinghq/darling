project(libc-darwin)

cmake_minimum_required(VERSION 2.4.0)

set(darwin_sources
	#_dirhelper.c # Doesn't build, not needed now
	kvm.c
	libproc.c
	MKGetTimeBaseInfo.c
	proc_listpidspath.c)

add_library(libc-darwin OBJECT ${darwin_sources})
