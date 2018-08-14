project(bsdln)

add_executable(bsdln
	ln.c
)
target_link_libraries(bsdln bsd)
