project(menu)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(menu_sources
	m_attribs.c
	m_cursor.c
	m_driver.c
	m_format.c
	m_global.c
	m_hook.c
	m_item_cur.c
	m_item_nam.c
	m_item_new.c
	m_item_opt.c
	m_items.c
	m_item_top.c
	m_item_use.c
	m_item_val.c
	m_item_vis.c
	m_new.c
	m_opts.c
	m_pad.c
	m_pattern.c
	m_post.c
	m_req_name.c
	m_scale.c
	m_spacing.c
	m_sub.c
	m_trace.c
	m_userptr.c
	m_win.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/libmenu.5.4.dylib")
add_darling_library(menu SHARED ${menu_sources})
target_link_libraries(menu system ncurses)

install(TARGETS menu DESTINATION libexec/darling/usr/lib)
