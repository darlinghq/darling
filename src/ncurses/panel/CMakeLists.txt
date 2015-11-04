project(panel)

cmake_minimum_required(VERSION 2.4.0)

set(panel_sources
	p_above.c
	panel.c
	p_below.c
	p_bottom.c
	p_delete.c
	p_hidden.c
	p_hide.c
	p_move.c
	p_new.c
	p_replace.c
	p_show.c
	p_top.c
	p_update.c
	p_user.c
	p_win.c
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${SUFFIX}/darling")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(panel SHARED ${panel_sources})
target_link_libraries(panel system)

install(TARGETS panel DESTINATION lib${SUFFIX}/darling)
