project(libc-posix1e)

cmake_minimum_required(VERSION 2.4.0)
include_directories(${DARLING_TOP_DIRECTORY}/src/libinfo/membership.subproj)

set(posix1e_sources
	acl.c
	acl_entry.c
	acl_file.c
	acl_flag.c
	acl_perm.c
	acl_translate.c # doesn't build, ducted
)

add_library(libc-posix1e OBJECT ${posix1e_sources})
