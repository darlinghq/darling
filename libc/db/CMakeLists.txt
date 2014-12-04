project(libc-db)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c89")

set(db_sources btree/FreeBSD/bt_close.c
	btree/FreeBSD/bt_conv.c
	btree/FreeBSD/bt_debug.c
	btree/FreeBSD/bt_delete.c
	btree/FreeBSD/bt_get.c
	btree/FreeBSD/bt_open.c
	btree/FreeBSD/bt_overflow.c
	btree/FreeBSD/bt_page.c
	btree/FreeBSD/bt_put.c
	btree/FreeBSD/bt_search.c
	btree/FreeBSD/bt_seq.c
	btree/FreeBSD/bt_split.c
	btree/FreeBSD/bt_utils.c)

set(db_sources ${db_sources}
	db/FreeBSD/db.c)

set(db_sources ${db_sources}
	hash/FreeBSD/hash_bigkey.c
	hash/FreeBSD/hash_buf.c
	hash/FreeBSD/hash.c
	hash/FreeBSD/hash_func.c
	hash/FreeBSD/hash_log2.c
	hash/FreeBSD/hash_page.c
	hash/FreeBSD/ndbm.c)

set(db_sources ${db_sources}
	recno/FreeBSD/rec_close.c
	recno/FreeBSD/rec_delete.c
	recno/FreeBSD/rec_get.c
	recno/FreeBSD/rec_open.c
	recno/FreeBSD/rec_put.c
	recno/FreeBSD/rec_search.c
	recno/FreeBSD/rec_seq.c
	recno/FreeBSD/rec_utils.c)

add_library(libc-db OBJECT ${db_sources})

