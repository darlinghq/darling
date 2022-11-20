project(libacm)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_darling_static_library(acm SOURCES acmstub.c)