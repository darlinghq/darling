project(unwind-darwin)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set (unwind_SRCS
#	emutls.c
	unwind-c.c
	unwind-compat.c
	unwind-dw2-fde-darwin.c
	unwind-dw2-fde-compat.c
	unwind-dw2.c
	unwind-sjlj.c
)

add_library(unwind-darwin SHARED ${unwind_SRCS})
#target_link_libraries(unwind-darwin -Tlink.cmd)
install(TARGETS unwind-darwin DESTINATION lib)
