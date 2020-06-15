project(buildtools)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../external/cctools-port/cctools/include)

add_executable(getuuid getuuid.c)
add_executable(elfdep elfdep.c)

#add_subdirectory(Rez)
