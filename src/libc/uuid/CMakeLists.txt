project(libc-uuid)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

set(uuid_sources
	uuidsrc/clear.c
	uuidsrc/compare.c
	uuidsrc/copy.c
	uuidsrc/gen_uuid.c
	uuidsrc/pack.c
	uuidsrc/parse.c
	uuidsrc/isnull.c
	uuidsrc/unpack.c
	uuidsrc/unparse.c)

add_library(libc-uuid OBJECT ${uuid_sources})
