#
# Copyright (c) 2011 Marius Zwicker <marius@mlba-team.de>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

#files
set(API api/test.c)
set(WITEM_CACHE witem_cache/test.c)
set(LATENCY latency/latency.c latency/latency.h)


#includes
include_directories(
	../include
)

if(UNIX)
    add_definitions(
        -DNO_CONFIG_H
    )
endif()

add_executable(test_api_pthread_workqueue ${API})
target_link_libraries(test_api_pthread_workqueue pthread_workqueue)
set_target_properties(test_api_pthread_workqueue PROPERTIES DEBUG_POSTFIX "D")

add_executable(test_latency_pthread_workqueue ${LATENCY})
target_link_libraries(test_latency_pthread_workqueue pthread_workqueue)

if(NOT WIN32)

        #add_executable(test_witem_cache_pthread_workqueue ${WITEM_CACHE})
        #target_link_libraries(test_witem_cache_pthread_workqueue pthreads_workqueue)

endif()
