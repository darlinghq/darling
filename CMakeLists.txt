project(darling)

cmake_minimum_required(VERSION 3.0)
set(DARLING_TOP_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

enable_language(ASM_NASM)
add_subdirectory(src)

install(FILES etc/dylib.conf DESTINATION /etc/darling)

