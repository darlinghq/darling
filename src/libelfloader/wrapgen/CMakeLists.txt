project(wrapgen)

cmake_minimum_required(VERSION 3.10)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

add_executable(wrapgen wrapgen.cpp)
target_link_libraries(wrapgen dl)

