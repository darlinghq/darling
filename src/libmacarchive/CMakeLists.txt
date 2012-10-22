project(macarchive)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
        cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

set(macarchive_SRCS
        FileStream.cpp
        MacBinaryStream.cpp
        DMGStream.cpp
        adc.c
)

add_library(macarchive SHARED ${macarchive_SRCS})
target_link_libraries(macarchive -lssl -lz -lbz2)

install(TARGETS macarchive DESTINATION "lib{SUFFIX}/darling")


