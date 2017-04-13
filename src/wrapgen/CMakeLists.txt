project(wrapgen)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(wrapgen wrapgen.cpp)
target_link_libraries(wrapgen dl)
