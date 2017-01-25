project(panel)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

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

set(DYLIB_INSTALL_NAME "/usr/lib/libpanel.5.4.dylib")
add_darling_library(panel SHARED ${panel_sources})
target_link_libraries(panel system ncurses)
set_target_properties(panel PROPERTIES OUTPUT_NAME "panel.5.4")
make_fat(panel)

InstallSymlink("libpanel.5.4.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libpanel.dylib")

install(TARGETS panel DESTINATION libexec/darling/usr/lib)
