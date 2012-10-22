project(macbinary)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
        cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(macbinary_SRCS
        macbinary.cpp
)

add_executable(macbinary ${macbinary_SRCS})

install(TARGETS macbinary DESTINATION bin)


