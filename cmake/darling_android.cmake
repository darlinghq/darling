# Copyright (C) 2026 Lubos Dolezel
#
# Install layout for Android hosts (Termux and the like), where the host has no /tmp and Darling's
# install prefix is an application directory outside the usual top-level names.

include_guard(GLOBAL)

# darling_host_is_android(<variable>)
#
# Whether Darling is being built for an Android host. Decided from the compiler (it defines
# __ANDROID__ for Bionic targets); DARLING_HOST_ANDROID=ON/OFF on the command line overrides it.
# Nothing is probed in the file system of the build machine.
function(darling_host_is_android variable)
	if (DEFINED DARLING_HOST_ANDROID)
		set(${variable} ${DARLING_HOST_ANDROID} PARENT_SCOPE)
		return()
	endif()
	include(CheckCSourceCompiles)
	check_c_source_compiles("
#ifndef __ANDROID__
#error not Android
#endif
int main(void) { return 0; }" DARLING_COMPILER_TARGETS_ANDROID)
	set(${variable} ${DARLING_COMPILER_TARGETS_ANDROID} PARENT_SCOPE)
endfunction()

# darling_host_root_segment(<install prefix> <variable>)
#
# The first path component of the install prefix, when the guest needs a link of that name to the host's
# directory of that name (so that the host paths of the installation resolve inside the guest, for
# example /data on Android). Empty when the prefix is in a directory the guest has itself, or when it
# is not an absolute path.
function(darling_host_root_segment prefix variable)
	set(${variable} "" PARENT_SCOPE)
	if (NOT prefix MATCHES "^/([^/]+)")
		return()
	endif()
	set(segment "${CMAKE_MATCH_1}")
	if (segment MATCHES "^(usr|bin|sbin|etc|var|dev|proc|System|Library|Volumes|Applications|Users|tmp|private|cores|opt|home|mnt|root)$")
		return()
	endif()
	set(${variable} "${segment}" PARENT_SCOPE)
endfunction()
