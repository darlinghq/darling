# Copyright (C) 2026 Lubos Dolezel
#
# Run with: cmake -P tests/android-layout.cmake
# Checks the helpers of cmake/darling_android.cmake that decide the Android install layout.
get_filename_component(here "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
include("${here}/../cmake/darling_android.cmake")

set(failures 0)
function(check_segment prefix expected)
	darling_host_root_segment("${prefix}" actual)
	if (NOT actual STREQUAL expected)
		message(SEND_ERROR "darling_host_root_segment(${prefix}) = '${actual}', expected '${expected}'")
	endif()
endfunction()

# a Termux-style application directory: the first component needs a link in the guest
check_segment("/data/data/org.example/files/usr" "data")
check_segment("/storage/emulated/0/darling" "storage")
# prefixes in directories the guest has itself need none
check_segment("/usr/local" "")
check_segment("/usr" "")
check_segment("/opt/darling" "")
check_segment("/home/user/darling" "")
check_segment("/tmp/stage" "")
check_segment("/private/x" "")
check_segment("/Volumes/x" "")
# not absolute, or nothing to take
check_segment("relative/path" "")
check_segment("/" "")
check_segment("" "")

# an explicit setting wins over detection, either way
set(DARLING_HOST_ANDROID ON)
darling_host_is_android(result)
if (NOT result)
	message(SEND_ERROR "DARLING_HOST_ANDROID=ON was not honoured")
endif()
set(DARLING_HOST_ANDROID OFF)
darling_host_is_android(result)
if (result)
	message(SEND_ERROR "DARLING_HOST_ANDROID=OFF was not honoured")
endif()
message(STATUS "android-layout checks done")
